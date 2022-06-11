#include <je2be.hpp>

#include "CommandID.h"
#include "Constants.h"
#include "File.h"
#include "TaskbarProgress.h"
#include "TemporaryDirectory.h"
#include "component/TextButton.h"
#include "component/x2b/X2BConvertProgress.h"

using namespace juce;

namespace je2be::desktop::component::x2b {

class X2BConvertProgress::Updater : public AsyncUpdater {
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
    if (!target)
      return;
    for (auto const &e : copy) {
      target->onProgressUpdate(e.fPhase, e.fDone, e.fTotal, fStatus);
    }
  }

  void complete(Status status) {
    fStatus = status;
  }

  std::atomic<X2BConvertProgress *> fTarget;

private:
  std::deque<Entry> fQueue;
  std::mutex fMut;
  Status fStatus;
};

class X2BWorkerThread : public Thread, public je2be::box360::Progress, public je2be::tobe::Progress {
public:
  X2BWorkerThread(File input, File output, std::shared_ptr<X2BConvertProgress::Updater> updater, File tempRoot)
      : Thread("je2be::desktop::component::x2b::X2BWorkerThread"), fInput(input), fOutput(output), fUpdater(updater), fTempRoot(tempRoot) {}

  void run() override {
    using Phase = je2be::desktop::component::x2b::X2BConvertProgress::Phase;

    try {
      unsafeRun();
    } catch (...) {
      fUpdater->complete(Status(Error(__FILE__, __LINE__)));
      fUpdater->trigger(Phase::Error, 1, 1);
    }
  }

  void unsafeRun() {
    using Phase = je2be::desktop::component::x2b::X2BConvertProgress::Phase;

    juce::Uuid u;
    File javaIntermediateDirectory = fTempRoot.getChildFile(u.toDashedString());
    if (auto result = javaIntermediateDirectory.createDirectory(); !result.ok()) {
      fUpdater->complete(Status(Error(__FILE__, __LINE__)));
      fUpdater->trigger(Phase::Error, 1, 1);
      return;
    }
    {
      je2be::box360::Options o;
      o.fTempDirectory = PathFromFile(fTempRoot);
      auto status = je2be::box360::Converter::Run(PathFromFile(fInput), PathFromFile(javaIntermediateDirectory), std::thread::hardware_concurrency(), o, this);
      if (!status.ok()) {
        fUpdater->complete(status);
        fUpdater->trigger(Phase::Error, 1, 1);
        return;
      }
    }
    if (threadShouldExit()) {
      fUpdater->trigger(Phase::Error, 1, 1);
      return;
    }
    {
      je2be::tobe::Options o;
      o.fTempDirectory = PathFromFile(fTempRoot);
      je2be::tobe::Converter c(PathFromFile(javaIntermediateDirectory), PathFromFile(fOutput), o);
      auto status = c.run(std::thread::hardware_concurrency(), this);
      fUpdater->complete(status);
    }
    fUpdater->trigger(Phase::Done, 1, 1);
  }

  bool report(double done, double total) override {
    using Phase = je2be::desktop::component::x2b::X2BConvertProgress::Phase;

    fUpdater->trigger(Phase::XboxToJavaConversion, done, total);
    return !threadShouldExit();
  }

  bool report(je2be::tobe::Progress::Phase phase, double done, double total) override {
    using Phase = je2be::desktop::component::x2b::X2BConvertProgress::Phase;

    switch (phase) {
    case je2be::tobe::Progress::Phase::Convert:
      fUpdater->trigger(Phase::JavaToBedrockConversion, done, total);
      break;
    case je2be::tobe::Progress::Phase::LevelDbCompaction:
      fUpdater->trigger(Phase::JavaToBedrockCompaction, done, total);
      break;
    }
    return !threadShouldExit();
  }

private:
  File const fInput;
  File const fOutput;
  File const fTempRoot;
  std::shared_ptr<X2BConvertProgress::Updater> fUpdater;
};

X2BConvertProgress::X2BConvertProgress(X2BConfigState const &configState) : fConfigState(configState) {
  auto width = kWindowWidth;
  auto height = kWindowHeight;
  setSize(width, height);

  fCancelButton.reset(new component::TextButton(TRANS("Cancel")));
  fCancelButton->setBounds(kMargin, height - kMargin - kButtonBaseHeight, kButtonMinWidth, kButtonBaseHeight);
  fCancelButton->onClick = [this]() { onCancelButtonClicked(); };
  addAndMakeVisible(*fCancelButton);

  int y = kMargin;
  fLabel.reset(new Label("", TRANS("Extracting...")));
  fLabel->setBounds(kMargin, y, width - 2 * kMargin, kButtonBaseHeight);
  fLabel->setJustificationType(Justification::topLeft);
  addAndMakeVisible(*fLabel);
  int errorMessageY = y + fLabel->getHeight();
  y += fLabel->getHeight() + kMargin;

  fXbox360ToJavaConversionProgressBar.reset(new ProgressBar(fXbox360ToJavaConversionProgress));
  fXbox360ToJavaConversionProgressBar->setBounds(kMargin, y, width - 2 * kMargin, kButtonBaseHeight);
  fXbox360ToJavaConversionProgressBar->setTextToDisplay("Extraction: ");
  addAndMakeVisible(*fXbox360ToJavaConversionProgressBar);
  y += fXbox360ToJavaConversionProgressBar->getHeight() + kMargin;

  fJavaToBedrockConversionProgressBar.reset(new ProgressBar(fJavaToBedrockConversionProgress));
  fJavaToBedrockConversionProgressBar->setBounds(kMargin, y, width - 2 * kMargin, kButtonBaseHeight);
  fJavaToBedrockConversionProgressBar->setTextToDisplay("Conversion: ");
  addAndMakeVisible(*fJavaToBedrockConversionProgressBar);
  y += fJavaToBedrockConversionProgressBar->getHeight() + kMargin;

  fJavaToBedrockCompactionProgressBar.reset(new ProgressBar(fJavaToBedrockCompactionProgress));
  fJavaToBedrockCompactionProgressBar->setBounds(kMargin, y, width - 2 * kMargin, kButtonBaseHeight);
  fJavaToBedrockCompactionProgressBar->setTextToDisplay("LevelDB Compaction: ");
  addAndMakeVisible(*fJavaToBedrockCompactionProgressBar);

  fErrorMessage.reset(new TextEditor());
  fErrorMessage->setBounds(kMargin, errorMessageY, width - 2 * kMargin, fCancelButton->getY() - kMargin - errorMessageY);
  fErrorMessage->setEnabled(false);
  fErrorMessage->setMultiLine(true);
  fErrorMessage->setColour(TextEditor::backgroundColourId, findColour(Label::backgroundColourId));
  addChildComponent(*fErrorMessage);

  fTaskbarProgress.reset(new TaskbarProgress());

  File temp = TemporaryDirectory::EnsureExisting();
  juce::Uuid u;
  File outputDir = temp.getChildFile(u.toDashedString());
  outputDir.createDirectory();
  fOutputDirectory = outputDir;

  fUpdater = std::make_shared<Updater>();
  fUpdater->fTarget.store(this);

  fThread.reset(new X2BWorkerThread(configState.fInputState.fInput, outputDir, fUpdater, temp));
  fThread->startThread();
}

X2BConvertProgress::~X2BConvertProgress() {
  fThread->stopThread(-1);
  fTaskbarProgress->setState(TaskbarProgress::State::NoProgress);
}

void X2BConvertProgress::paint(juce::Graphics &g) {}

void X2BConvertProgress::onCancelButtonClicked() {
  if (fFailed) {
    JUCEApplication::getInstance()->invoke(commands::toModeSelect, true);
  } else {
    fCancelRequested = true;
    fCancelButton->setEnabled(false);
    fCommandWhenFinished = commands::toXbox360ToBedrockConfig;
    fThread->signalThreadShouldExit();
    fXbox360ToJavaConversionProgress = -1;
    fXbox360ToJavaConversionProgressBar->setVisible(true);
    fXbox360ToJavaConversionProgressBar->setTextToDisplay({});
    fJavaToBedrockConversionProgressBar->setVisible(false);
    fJavaToBedrockCompactionProgressBar->setVisible(false);
    fLabel->setText(TRANS("Waiting for the worker thread to finish"), dontSendNotification);
  }
}

void X2BConvertProgress::onProgressUpdate(Phase phase, double done, double total, Status st) {
  double progress = done / total;
  fFailed = !st.ok();

  if (phase == Phase::Done && !fCancelRequested) {
    if (fCommandWhenFinished != commands::toChooseBedrockOutput && fOutputDirectory.exists()) {
      TemporaryDirectory::QueueDeletingDirectory(fOutputDirectory);
    }
    if (st.ok()) {
      fState = BedrockConvertedState(fConfigState.fInputState.fWorldName, fOutputDirectory);
      JUCEApplication::getInstance()->invoke(fCommandWhenFinished, true);
    }
    if (fFailed) {
      fTaskbarProgress->setState(TaskbarProgress::State::Error);
    } else {
      fTaskbarProgress->setState(TaskbarProgress::State::NoProgress);
    }
  } else if (phase == Phase::JavaToBedrockCompaction && !fCancelRequested) {
    fLabel->setText(TRANS("LevelDB compaction"), dontSendNotification);
    fXbox360ToJavaConversionProgress = 1;
    fJavaToBedrockConversionProgress = 1;
    if (progress > 0) {
      fJavaToBedrockCompactionProgress = progress;
      fTaskbarProgress->update(2.0 / 3.0 + progress / 3.0);
    } else {
      fJavaToBedrockCompactionProgress = -1;
      fTaskbarProgress->update(2.0 / 3.0);
    }
    fTaskbarProgress->setState(TaskbarProgress::State::Normal);
  } else if (phase == Phase::JavaToBedrockConversion && !fCancelRequested) {
    fLabel->setText(TRANS("Converting..."), dontSendNotification);
    fXbox360ToJavaConversionProgress = 1;
    if (progress > 0) {
      fJavaToBedrockConversionProgress = progress;
      fTaskbarProgress->update(1.0 / 3.0 + progress / 3.0);
    } else {
      fJavaToBedrockConversionProgress = -1;
      fTaskbarProgress->update(1.0 / 3.0);
    }
    fTaskbarProgress->setState(TaskbarProgress::State::Normal);
    fJavaToBedrockCompactionProgress = 0;
  } else if (phase == Phase::XboxToJavaConversion && !fCancelRequested) {
    if (progress > 0) {
      fXbox360ToJavaConversionProgress = progress;
      fTaskbarProgress->setState(TaskbarProgress::State::Normal);
      fTaskbarProgress->update(progress / 3.0);
    } else {
      fXbox360ToJavaConversionProgress = -1;
      fTaskbarProgress->setState(TaskbarProgress::State::Indeterminate);
    }
    fJavaToBedrockConversionProgress = 0;
    fJavaToBedrockCompactionProgress = 0;
  } else {
    fFailed = true;
  }
  if (fFailed) {
    fLabel->setText(TRANS("The conversion failed."), dontSendNotification);
    fLabel->setColour(Label::textColourId, kErrorTextColor);
    auto error = st.error();
    if (error) {
      juce::String message = juce::String(JUCE_APPLICATION_NAME_STRING) + " version " + JUCE_APPLICATION_VERSION_STRING;
      message += juce::String("\nFailed at file ") + error->fFile + ":" + std::to_string(error->fLine);
      fErrorMessage->setText(message);
      fErrorMessage->setVisible(true);
    }
    fCancelButton->setButtonText(TRANS("Back"));
    fCancelButton->setEnabled(true);
    fXbox360ToJavaConversionProgressBar->setVisible(false);
    fJavaToBedrockConversionProgressBar->setVisible(false);
    fJavaToBedrockCompactionProgressBar->setVisible(false);
  }
}

} // namespace je2be::desktop::component::x2b
