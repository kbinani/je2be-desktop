#include "ConvertProgressComponent.h"
#include "CommandID.h"
#include "Constants.h"
#include "TemporaryDirectory.h"
#include <je2be.hpp>

using namespace juce;

class ConvertProgressComponent::Updater : public AsyncUpdater {
  struct Entry {
    int fPhase;
    double fDone;
    double fTotal;
  };

public:
  void trigger(int phase, double done, double total) {
    Entry entry;
    entry.fPhase = phase;
    entry.fDone = done;
    entry.fTotal = total;
    {
      std::lock_guard<std::mutex> lk(fMut);
      fQueue.push_back(entry);
    }
    triggerAsyncUpdate();
  }

  void handleAsyncUpdate() override {
    std::deque<Entry> copy;
    {
      std::lock_guard<std::mutex> lk(fMut);
      copy.swap(fQueue);
    }
    auto target = fTarget.load();
    if (!target)
      return;
    for (auto const &e : copy) {
      target->onProgressUpdate(e.fPhase, e.fDone, e.fTotal);
    }
  }

  void complete(j2b::Statistics stat) { fStat = stat; }

  std::atomic<ConvertProgressComponent *> fTarget;
  std::optional<j2b::Statistics> fStat;

private:
  std::deque<Entry> fQueue;
  std::mutex fMut;
};

class WorkerThread : public Thread, public j2b::Progress {
public:
  WorkerThread(File input, j2b::InputOption io, File output,
               j2b::OutputOption oo,
               std::shared_ptr<ConvertProgressComponent::Updater> updater)
      : Thread("j2b::gui::Convert"), fInput(input), fInputOption(io),
        fOutput(output), fOutputOption(oo), fUpdater(updater) {}

  void run() override {
    using namespace j2b;
    Converter c(fInput.getFullPathName().toWideCharPointer(), fInputOption,
                fOutput.getFullPathName().toWideCharPointer(), fOutputOption);
    try {
      auto stat = c.run(std::thread::hardware_concurrency(), this);
      if (stat) {
        fUpdater->complete(*stat);
      }
      fUpdater->trigger(2, 1, 1);
    } catch (std::filesystem::filesystem_error &e) {
      fUpdater->trigger(-1, 1, 1);
    } catch (...) {
      fUpdater->trigger(-1, 1, 1);
    }
  }

  bool report(j2b::Progress::Phase phase, double done, double total) override {
    int p = 0;
    switch (phase) {
    case j2b::Progress::Phase::Convert:
      p = 0;
      break;
    case j2b::Progress::Phase::LevelDbCompaction:
      p = 1;
      break;
    }
    fUpdater->trigger(p, done, total);
    return !threadShouldExit();
  }

private:
  File const fInput;
  j2b::InputOption const fInputOption;
  File const fOutput;
  j2b::OutputOption const fOutputOption;
  std::shared_ptr<ConvertProgressComponent::Updater> fUpdater;
};

static ConvertStatistics Import(j2b::Statistics stat) {
  ConvertStatistics ret;
  for (auto const &it : stat.fChunkDataVersions) {
    ret.fChunkDataVersions[it.first] = it.second;
  }
  ret.fNumChunks = stat.fNumChunks;
  ret.fNumBlockEntities = stat.fNumBlockEntities;
  ret.fNumEntities = stat.fNumEntities;
  return ret;
}

static String DimensionToString(j2b::Dimension dim) {
  switch (dim) {
  case j2b::Dimension::Overworld:
    return TRANS("Overworld");
  case j2b::Dimension::Nether:
    return TRANS("Nether");
  case j2b::Dimension::End:
    return TRANS("End");
  }
  return "Unknown";
}

ConvertProgressComponent::ConvertProgressComponent(
    ConfigState const &configState)
    : fState(configState) {
  auto width = kWindowWidth;
  auto height = kWindowHeight;
  setSize(width, height);

  fCancelButton.reset(new TextButton(TRANS("Cancel")));
  fCancelButton->setBounds(kMargin, height - kMargin - kButtonBaseHeight,
                           kButtonMinWidth, kButtonBaseHeight);
  fCancelButton->setMouseCursor(MouseCursor::PointingHandCursor);
  fCancelButton->onClick = [this]() { onCancelButtonClicked(); };
  addAndMakeVisible(*fCancelButton);

  int y = kMargin;
  fLabel.reset(new Label("", TRANS("Converting...")));
  fLabel->setBounds(kMargin, y, width - 2 * kMargin, kButtonBaseHeight);
  fLabel->setJustificationType(Justification::topLeft);
  addAndMakeVisible(*fLabel);
  y += fLabel->getHeight() + kMargin;

  fProgressBar.reset(new ProgressBar(fProgress));
  fProgressBar->setBounds(kMargin, y, width - 2 * kMargin, kButtonBaseHeight);
  addAndMakeVisible(*fProgressBar);

  fErrorMessage.reset(new TextEditor());
  fErrorMessage->setBounds(kMargin, y, width - 2 * kMargin,
                           fCancelButton->getY() - y - kMargin);
  fErrorMessage->setEnabled(false);
  fErrorMessage->setMultiLine(true);
  addChildComponent(*fErrorMessage);

  File temp = TemporaryDirectory::EnsureExisting();
  Uuid u;
  File outputDir = temp.getChildFile(u.toDashedString());
  outputDir.createDirectory();
  fState.fOutputDirectory = outputDir;

  fUpdater = std::make_shared<Updater>();
  fUpdater->fTarget.store(this);

  j2b::InputOption io;
  if (fState.fConfigState.fStructure ==
      ConfigState::DirectoryStructure::Paper) {
    io.fLevelDirectoryStructure = j2b::LevelDirectoryStructure::Paper;
  }
  fThread.reset(new WorkerThread(*configState.fInputState.fInputDirectory, io,
                                 fState.fOutputDirectory, {}, fUpdater));
  fThread->startThread();
}

ConvertProgressComponent::~ConvertProgressComponent() {
  fThread->stopThread(-1);
}

void ConvertProgressComponent::paint(juce::Graphics &g) {}

void ConvertProgressComponent::onCancelButtonClicked() {
  if (fFailed) {
    JUCEApplication::getInstance()->invoke(gui::toChooseInput, true);
  } else {
    fCancelButton->setEnabled(false);
    fCancelButton->setMouseCursor(MouseCursor::NormalCursor);
    fCommandWhenFinished = gui::toConfig;
    fThread->signalThreadShouldExit();
    fProgress = -1;
    fLabel->setText(TRANS("Waiting for the worker thread to finish"),
                    dontSendNotification);
  }
}

void ConvertProgressComponent::onProgressUpdate(int phase, double done,
                                                double total) {
  if (phase == 2) {
    if (fCommandWhenFinished != gui::toChooseOutput &&
        fState.fOutputDirectory.exists()) {
      fState.fOutputDirectory.deleteRecursively();
    }
    auto stat = fUpdater->fStat;
    if (stat) {
      fState.fStat = Import(*stat);
      if (stat->fErrors.empty()) {
        JUCEApplication::getInstance()->invoke(fCommandWhenFinished, true);
      } else {
        fFailed = true;
        String msg = "Failed chunks:\n";
        for (auto const &it : stat->fErrors) {
          int32_t regionX = it.fChunkX >> 5;
          int32_t regionZ = it.fChunkZ >> 5;
          msg += "    " + DimensionToString(it.fDim) + " chunk (" +
                 String(it.fChunkX) + ", " + String(it.fChunkZ) + ") at r." +
                 String(regionX) + "." + String(regionZ) + ".mca\n";
        }
        fErrorMessage->setText(msg);
        fErrorMessage->setVisible(true);
      }
    } else {
      fFailed = true;
    }
  } else if (phase == 1) {
    fLabel->setText(TRANS("LevelDB compaction"), dontSendNotification);
    fProgress = -1;
  } else if (phase == 0) {
    if (fProgress >= 0) {
      fProgress = done / total;
    }
  } else {
    fFailed = true;
  }
  if (fFailed) {
    fLabel->setText(TRANS("The conversion failed."), dontSendNotification);
    fLabel->setColour(Label::textColourId, kErrorTextColor);
    fCancelButton->setButtonText(TRANS("Back"));
    fProgressBar->setVisible(false);
  }
}
