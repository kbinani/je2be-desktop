#include <je2be.hpp>

#include "CommandID.h"
#include "Constants.h"
#include "File.h"
#include "J2BConvertProgressComponent.h"
#include "TaskbarProgress.h"
#include "TemporaryDirectory.h"

using namespace juce;

namespace je2be::gui::j2b {

class J2BConvertProgressComponent::Updater : public AsyncUpdater {
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

  void complete(je2be::tobe::Statistics stat) {
    fStat = stat;
  }

  std::atomic<J2BConvertProgressComponent *> fTarget;
  std::optional<je2be::tobe::Statistics> fStat;

private:
  std::deque<Entry> fQueue;
  std::mutex fMut;
};

class J2BWorkerThread : public Thread, public je2be::tobe::Progress {
public:
  J2BWorkerThread(File input, File output, je2be::tobe::Options opt,
                  std::shared_ptr<J2BConvertProgressComponent::Updater> updater)
      : Thread("je2be::gui::J2BConvert"), fInput(input), fOutput(output), fOptions(opt), fUpdater(updater) {}

  void run() override {
    je2be::tobe::Converter c(PathFromFile(fInput), PathFromFile(fOutput), fOptions);
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

  bool report(je2be::tobe::Progress::Phase phase, double done, double total) override {
    int p = 0;
    switch (phase) {
    case je2be::tobe::Progress::Phase::Convert:
      p = 0;
      break;
    case je2be::tobe::Progress::Phase::LevelDbCompaction:
      p = 1;
      break;
    }
    fUpdater->trigger(p, done, total);
    return !threadShouldExit();
  }

private:
  File const fInput;
  File const fOutput;
  je2be::tobe::Options const fOptions;
  std::shared_ptr<J2BConvertProgressComponent::Updater> fUpdater;
};

static J2BConvertStatistics Import(je2be::tobe::Statistics stat) {
  J2BConvertStatistics ret;
  for (auto const &it : stat.fChunkDataVersions) {
    ret.fChunkDataVersions[it.first] = it.second;
  }
  ret.fNumChunks = stat.fNumChunks;
  ret.fNumBlockEntities = stat.fNumBlockEntities;
  ret.fNumEntities = stat.fNumEntities;
  return ret;
}

static String DimensionToString(mcfile::Dimension dim) {
  switch (dim) {
  case mcfile::Dimension::Overworld:
    return TRANS("Overworld");
  case mcfile::Dimension::Nether:
    return TRANS("Nether");
  case mcfile::Dimension::End:
    return TRANS("End");
  }
  return "Unknown";
}

J2BConvertProgressComponent::J2BConvertProgressComponent(J2BConfigState const &configState) : fState(configState) {
  auto width = kWindowWidth;
  auto height = kWindowHeight;
  setSize(width, height);

  fCancelButton.reset(new TextButtonComponent(TRANS("Cancel")));
  fCancelButton->setBounds(kMargin, height - kMargin - kButtonBaseHeight, kButtonMinWidth, kButtonBaseHeight);
  fCancelButton->onClick = [this]() { onCancelButtonClicked(); };
  addAndMakeVisible(*fCancelButton);

  int y = kMargin;
  fLabel.reset(new Label("", TRANS("Converting...")));
  fLabel->setBounds(kMargin, y, width - 2 * kMargin, kButtonBaseHeight);
  fLabel->setJustificationType(Justification::topLeft);
  addAndMakeVisible(*fLabel);
  y += fLabel->getHeight() + kMargin;
  int errorMessageY = y;

  fConversionProgressBar.reset(new ProgressBar(fConversionProgress));
  fConversionProgressBar->setBounds(kMargin, y, width - 2 * kMargin, kButtonBaseHeight);
  fConversionProgressBar->setTextToDisplay("Conversion: ");
  addAndMakeVisible(*fConversionProgressBar);
  y += fConversionProgressBar->getHeight() + kMargin;

  fCompactionProgressBar.reset(new ProgressBar(fCompactionProgress));
  fCompactionProgressBar->setBounds(kMargin, y, width - 2 * kMargin, kButtonBaseHeight);
  fCompactionProgressBar->setTextToDisplay("LevelDB Compaction: ");
  addAndMakeVisible(*fCompactionProgressBar);

  fErrorMessage.reset(new TextEditor());
  fErrorMessage->setBounds(kMargin, errorMessageY, width - 2 * kMargin, fCancelButton->getY() - y - kMargin);
  fErrorMessage->setEnabled(false);
  fErrorMessage->setMultiLine(true);
  addChildComponent(*fErrorMessage);

  fTaskbarProgress.reset(new TaskbarProgress());

  File temp = TemporaryDirectory::EnsureExisting();
  juce::Uuid u;
  File outputDir = temp.getChildFile(u.toDashedString());
  outputDir.createDirectory();
  fState.fOutputDirectory = outputDir;

  fUpdater = std::make_shared<Updater>();
  fUpdater->fTarget.store(this);

  je2be::tobe::Options opt;
  if (fState.fConfigState.fStructure == J2BConfigState::DirectoryStructure::Paper) {
    opt.fLevelDirectoryStructure = je2be::LevelDirectoryStructure::Paper;
  }
  fThread.reset(new J2BWorkerThread(*configState.fInputState.fInputDirectory, fState.fOutputDirectory, opt, fUpdater));
  fThread->startThread();
}

J2BConvertProgressComponent::~J2BConvertProgressComponent() {
  fThread->stopThread(-1);
  fTaskbarProgress->setState(TaskbarProgress::State::NoProgress);
}

void J2BConvertProgressComponent::paint(juce::Graphics &g) {}

void J2BConvertProgressComponent::onCancelButtonClicked() {
  if (fFailed) {
    JUCEApplication::getInstance()->invoke(gui::toJ2BChooseInput, true);
  } else {
    fCancelButton->setEnabled(false);
    fCommandWhenFinished = gui::toJ2BConfig;
    fThread->signalThreadShouldExit();
    fConversionProgress = -1;
    fLabel->setText(TRANS("Waiting for the worker thread to finish"), dontSendNotification);
  }
}

void J2BConvertProgressComponent::onProgressUpdate(int phase, double done, double total) {
  double weightConversion = 0.67;
  double weightCompaction = 1 - weightConversion;

  if (phase == 2) {
    if (fCommandWhenFinished != gui::toJ2BChooseOutput && fState.fOutputDirectory.exists()) {
      TemporaryDirectory::QueueDeletingDirectory(fState.fOutputDirectory);
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
    if (fFailed) {
      fTaskbarProgress->setState(TaskbarProgress::State::Error);
    } else {
      fTaskbarProgress->setState(TaskbarProgress::State::NoProgress);
    }
  } else if (phase == 1) {
    fLabel->setText(TRANS("LevelDB compaction"), dontSendNotification);
    double progress = done / total;
    fConversionProgress = 1;
    fCompactionProgress = progress;
    fTaskbarProgress->setState(TaskbarProgress::State::Normal);
    fTaskbarProgress->update(weightConversion + progress * weightCompaction);
  } else if (phase == 0) {
    if (fConversionProgress >= 0) {
      double progress = done / total;
      fConversionProgress = progress;
      fCompactionProgress = 0;
      fTaskbarProgress->setState(TaskbarProgress::State::Normal);
      fTaskbarProgress->update(progress * weightConversion);
    }
  } else {
    fFailed = true;
  }
  if (fFailed) {
    fLabel->setText(TRANS("The conversion failed."), dontSendNotification);
    fLabel->setColour(Label::textColourId, kErrorTextColor);
    fCancelButton->setButtonText(TRANS("Back"));
    fConversionProgressBar->setVisible(false);
    fCompactionProgressBar->setVisible(false);
  }
}

} // namespace je2be::gui::j2b
