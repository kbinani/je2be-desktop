#include "B2JConvertProgressComponent.h"
#include "CommandID.h"
#include "Constants.h"
#include "TaskbarProgress.h"
#include "TemporaryDirectory.h"
#include <je2be.hpp>

using namespace juce;

namespace je2be::gui {

class B2JConvertProgressComponent::Updater : public AsyncUpdater {
  struct Entry {
    Phase fPhase;
    double fDone;
    double fTotal;
  };

public:
  void trigger(Phase phase, double done, double total) {
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
    if (!target) {
      return;
    }
    for (auto const &e : copy) {
      target->onProgressUpdate(e.fPhase, e.fDone, e.fTotal);
    }
  }

  void complete(bool ok) {
    fOk = ok;
  }

  std::atomic<B2JConvertProgressComponent *> fTarget;
  bool fOk = false;

private:
  std::deque<Entry> fQueue;
  std::mutex fMut;
};

class B2JWorkerThread : public Thread, public je2be::toje::Progress {
public:
  B2JWorkerThread(File input, File output,
                  std::shared_ptr<B2JConvertProgressComponent::Updater> updater)
      : Thread("je2be::gui::B2JConvert"), fInput(input),
        fOutput(output), fUpdater(updater) {}

  void run() override {
    try {
      unsafeRun();
    } catch (std::filesystem::filesystem_error &e) {
      fUpdater->trigger(B2JConvertProgressComponent::Phase::Error, 1, 1);
    } catch (...) {
      fUpdater->trigger(B2JConvertProgressComponent::Phase::Error, 1, 1);
    }
  }

  void unsafeRun() {
    File input = fInput;
    if (fInput.isDirectory()) {
      fUpdater->trigger(B2JConvertProgressComponent::Phase::Unzip, 1, 1);
    } else {
      fUpdater->trigger(B2JConvertProgressComponent::Phase::Unzip, 0, 1);
      File sessionTempDir = TemporaryDirectory::EnsureExisting();
      juce::Uuid u;
      File temp = sessionTempDir.getChildFile(u.toDashedString());
      temp.createDirectory();
      ZipFile zip(fInput);
      int numEntries = zip.getNumEntries();
      fUpdater->trigger(B2JConvertProgressComponent::Phase::Unzip, 0, numEntries);
      for (int i = 0; i < numEntries; ++i) {
        auto result = zip.uncompressEntry(i, temp);
        if (result.failed()) {
          fUpdater->trigger(B2JConvertProgressComponent::Phase::Error, 1, 1);
          return;
        }
        fUpdater->trigger(B2JConvertProgressComponent::Phase::Unzip, i + 1, numEntries);
      }
      input = temp;
    }
    je2be::toje::Converter c(
#if defined(_WIN32)
        std::filesystem::path(input.getFullPathName().toWideCharPointer()),
        std::filesystem::path(fOutput.getFullPathName().toWideCharPointer())
#else
        std::filesystem::path(input.getFullPathName().toStdString()),
        std::filesystem::path(fOutput.getFullPathName().toStdString())
#endif
    );
    bool ok = c.run(std::thread::hardware_concurrency(), this);
    fUpdater->complete(ok);
    fUpdater->trigger(B2JConvertProgressComponent::Phase::Done, 1, 1);
  }

  bool report(je2be::toje::Progress::Phase phase, double done, double total) override {
    double offset = 0;
    switch (phase) {
    case je2be::toje::Progress::Phase::Dimension2:
      offset = 1.0 / 3.0;
      break;
    case je2be::toje::Progress::Phase::Dimension3:
      offset = 2.0 / 3.0;
      break;
    case je2be::toje::Progress::Phase::Dimension1:
    default:
      offset = 0;
      break;
    }
    fUpdater->trigger(B2JConvertProgressComponent::Phase::Conversion, offset + done / total / 3.0, 1.0);
    return !threadShouldExit();
  }

private:
  File const fInput;
  File const fOutput;
  std::shared_ptr<B2JConvertProgressComponent::Updater> fUpdater;
};

B2JConvertProgressComponent::B2JConvertProgressComponent(B2JConfigState const &configState) : fState(configState) {
  auto width = kWindowWidth;
  auto height = kWindowHeight;
  setSize(width, height);

  fUnzipProgress = -1;
  fConversionProgress = 0;

  fCancelButton.reset(new TextButton(TRANS("Cancel")));
  fCancelButton->setBounds(kMargin, height - kMargin - kButtonBaseHeight, kButtonMinWidth, kButtonBaseHeight);
  fCancelButton->setMouseCursor(MouseCursor::PointingHandCursor);
  fCancelButton->onClick = [this]() { onCancelButtonClicked(); };
  addAndMakeVisible(*fCancelButton);

  bool needsUnzip = !configState.fInputState.fInputFileOrDirectory->isDirectory();

  int y = kMargin;
  fLabel.reset(new Label("", TRANS("Unzipping...")));
  fLabel->setBounds(kMargin, y, width - 2 * kMargin, kButtonBaseHeight);
  fLabel->setJustificationType(Justification::topLeft);
  addAndMakeVisible(*fLabel);
  y += fLabel->getHeight() + kMargin;
  int errorMessageY = y;

  fUnzipProgressBar.reset(new ProgressBar(fUnzipProgress));
  fUnzipProgressBar->setBounds(kMargin, y, width - 2 * kMargin, kButtonBaseHeight);
  fUnzipProgressBar->setTextToDisplay("Unzip: ");
  addAndMakeVisible(*fUnzipProgressBar);
  if (needsUnzip) {
    y += fUnzipProgressBar->getHeight() + kMargin;
  } else {
    fUnzipProgressBar->setVisible(false);
    fLabel->setText(TRANS("Converting..."), dontSendNotification);
  }

  fConversionProgressBar.reset(new ProgressBar(fConversionProgress));
  fConversionProgressBar->setBounds(kMargin, y, width - 2 * kMargin, kButtonBaseHeight);
  fConversionProgressBar->setTextToDisplay("Conversion: ");
  addAndMakeVisible(*fConversionProgressBar);

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

  fThread.reset(new B2JWorkerThread(*configState.fInputState.fInputFileOrDirectory, fState.fOutputDirectory, fUpdater));
  fThread->startThread();
}

B2JConvertProgressComponent::~B2JConvertProgressComponent() {
  fThread->stopThread(-1);
  fTaskbarProgress->setState(TaskbarProgress::State::NoProgress);
}

void B2JConvertProgressComponent::paint(juce::Graphics &g) {}

void B2JConvertProgressComponent::onCancelButtonClicked() {
  if (fFailed) {
    JUCEApplication::getInstance()->invoke(gui::toB2JChooseInput, true);
  } else {
    fCancelButton->setEnabled(false);
    fCancelButton->setMouseCursor(MouseCursor::NormalCursor);
    fCommandWhenFinished = gui::toB2JConfig;
    fThread->signalThreadShouldExit();
    fConversionProgress = -1;
    fCancelRequested = true;
    fLabel->setText(TRANS("Waiting for the worker thread to finish"), dontSendNotification);
  }
}

void B2JConvertProgressComponent::onProgressUpdate(Phase phase, double done, double total) {
  double const weightUnzip = 0.5;
  double const weightConversion = 1 - weightUnzip;

  if (fCancelRequested) {
    return;
  }

  if (phase == Phase::Unzip) {
    double progress = done / total;
    fUnzipProgress = progress;
    if (progress >= 1) {
      fConversionProgress = -1;
    } else {
      fConversionProgress = 0;
    }
    fTaskbarProgress->setState(TaskbarProgress::State::Normal);
    fTaskbarProgress->update(progress * weightUnzip);
  } else if (phase == Phase::Conversion) {
    fLabel->setText(TRANS("Converting..."), dontSendNotification);
    double progress = done / total;
    if (progress > 0) {
      fConversionProgress = progress;
    }
    fUnzipProgress = 1;
    fTaskbarProgress->setState(TaskbarProgress::State::Normal);
    fTaskbarProgress->update(weightUnzip + progress * weightConversion);
  } else if (phase == Phase::Done) {
    if (fCommandWhenFinished != gui::toB2JChooseOutput && fState.fOutputDirectory.exists()) {
      fState.fOutputDirectory.deleteRecursively();
    }
    bool ok = fUpdater->fOk;
    if (ok) {
      JUCEApplication::getInstance()->invoke(fCommandWhenFinished, true);
    } else {
      fFailed = true;
    }
    if (fFailed) {
      fTaskbarProgress->setState(TaskbarProgress::State::Error);
    } else {
      fTaskbarProgress->setState(TaskbarProgress::State::NoProgress);
    }
  } else {
    fFailed = true;
  }
  if (fFailed) {
    fLabel->setText(TRANS("The conversion failed."), dontSendNotification);
    fLabel->setColour(Label::textColourId, kErrorTextColor);
    fCancelButton->setButtonText(TRANS("Back"));
    fCancelButton->setEnabled(true);
    fUnzipProgressBar->setVisible(false);
    fConversionProgressBar->setVisible(false);
  }
}

} // namespace je2be::gui
