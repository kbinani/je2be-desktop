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

class X2BWorkerThread : public Thread, public je2be::box360::Progress, public je2be::tobe::Progress {
public:
  X2BWorkerThread(File input, File output, std::shared_ptr<AsyncHandler<X2BConvertProgress::UpdateQueue>> updater, File tempRoot)
      : Thread("je2be::desktop::component::x2b::X2BWorkerThread"), fInput(input), fOutput(output), fUpdater(updater), fTempRoot(tempRoot) {}

  void run() override {
    using Phase = je2be::desktop::component::x2b::X2BConvertProgress::Phase;

    try {
      unsafeRun();
    } catch (std::filesystem::filesystem_error &e) {
      triggerError(Error(__FILE__, __LINE__, e.what()));
    } catch (std::exception &e) {
      triggerError(Error(__FILE__, __LINE__, e.what()));
    } catch (char const *what) {
      triggerError(Error(__FILE__, __LINE__, what));
    } catch (...) {
      triggerError(Error(__FILE__, __LINE__));
    }
  }

  void unsafeRun() {
    using Phase = je2be::desktop::component::x2b::X2BConvertProgress::Phase;

    juce::Uuid u;
    File javaIntermediateDirectory = fTempRoot.getChildFile(u.toDashedString());
    if (auto result = javaIntermediateDirectory.createDirectory(); !result.ok()) {
      triggerError(Error(__FILE__, __LINE__, result.getErrorMessage().toStdString()));
      return;
    }
    {
      je2be::box360::Options o;
      o.fTempDirectory = PathFromFile(fTempRoot);
      auto status = je2be::box360::Converter::Run(PathFromFile(fInput), PathFromFile(javaIntermediateDirectory), std::thread::hardware_concurrency(), o, this);
      if (!status.ok()) {
        triggerError(status);
        return;
      }
    }
    if (threadShouldExit()) {
      triggerError(Status::Ok());
      return;
    }
    {
      je2be::tobe::Options o;
      o.fTempDirectory = PathFromFile(fTempRoot);
      auto status = je2be::tobe::Converter::Run(PathFromFile(javaIntermediateDirectory), PathFromFile(fOutput), o, std::thread::hardware_concurrency(), this);
      triggerProgress(Phase::Done, 1, 1, status);
    }
  }

  bool report(double done, double total) override {
    using Phase = je2be::desktop::component::x2b::X2BConvertProgress::Phase;
    triggerProgress(Phase::XboxToJavaConversion, done, total);
    return !threadShouldExit();
  }

  bool report(je2be::tobe::Progress::Phase phase, double done, double total) override {
    using Phase = je2be::desktop::component::x2b::X2BConvertProgress::Phase;

    switch (phase) {
    case je2be::tobe::Progress::Phase::Convert:
      triggerProgress(Phase::JavaToBedrockConversion, done, total);
      break;
    case je2be::tobe::Progress::Phase::LevelDbCompaction:
      triggerProgress(Phase::JavaToBedrockCompaction, done, total);
      break;
    }
    return !threadShouldExit();
  }

  void triggerProgress(X2BConvertProgress::Phase phase, double done, double total, Status st = Status::Ok()) {
    X2BConvertProgress::UpdateQueue q;
    q.fStatus = st;
    q.fPhase = phase;
    q.fTotal = total;
    q.fDone = done;
    fUpdater->trigger(q);
  }

  void triggerError(Status st) {
    X2BConvertProgress::UpdateQueue q;
    q.fStatus = st;
    q.fPhase = X2BConvertProgress::Phase::Error;
    q.fTotal = 1;
    q.fDone = 1;
    fUpdater->trigger(q);
  }

private:
  File const fInput;
  File const fOutput;
  File const fTempRoot;
  std::shared_ptr<AsyncHandler<X2BConvertProgress::UpdateQueue>> fUpdater;
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

  fUpdater = std::make_shared<AsyncHandler<UpdateQueue>>([this](UpdateQueue q) {
    onProgressUpdate(q.fPhase, q.fDone, q.fTotal, q.fStatus);
  });

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
      message += juce::String("\nFailed: where: ") + error->fWhere.fFile + ":" + std::to_string(error->fWhere.fLine);
      if (!error->fWhat.empty()) {
        message += juce::String(", what: " + error->fWhat);
      }
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
