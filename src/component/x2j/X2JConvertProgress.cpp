#include <je2be.hpp>

#include "CommandID.h"
#include "Constants.h"
#include "File.h"
#include "TaskbarProgress.h"
#include "TemporaryDirectory.h"
#include "component/TextButton.h"
#include "component/x2j/X2JConvertProgress.h"

using namespace juce;

namespace je2be::desktop::component::x2j {

class X2JWorkerThread : public Thread, public je2be::box360::Progress {
public:
  X2JWorkerThread(File input, File output, je2be::box360::Options opt,
                  std::shared_ptr<AsyncHandler<X2JConvertProgress::UpdateQueue>> updater)
      : Thread("je2be::desktop::component::x2j::X2JWorkerThread"), fInput(input), fOutput(output), fOptions(opt), fUpdater(updater) {}

  void run() override {
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
    File sessionTempDir = TemporaryDirectory::EnsureExisting();
    juce::Uuid u;
    File temp = sessionTempDir.getChildFile(u.toDashedString());
    if (auto st = temp.createDirectory(); !st.ok()) {
      triggerError(Error(__FILE__, __LINE__, st.getErrorMessage().toStdString()));
      return;
    }
    defer {
      TemporaryDirectory::QueueDeletingDirectory(temp);
    };
    {
      auto status = je2be::box360::Converter::Run(PathFromFile(fInput), PathFromFile(fOutput), std::thread::hardware_concurrency(), fOptions, this);
      triggerProgress(X2JConvertProgress::Phase::Done, 1, 1, status);
    }
  }

  bool report(double done, double total) override {
    triggerProgress(X2JConvertProgress::Phase::Conversion, done / total, 1.0);
    return !threadShouldExit();
  }

  void triggerProgress(X2JConvertProgress::Phase phase, double done, double total, Status st = Status::Ok()) {
    X2JConvertProgress::UpdateQueue q;
    q.fStatus = st;
    q.fPhase = phase;
    q.fTotal = total;
    q.fDone = done;
    fUpdater->trigger(q);
  }

  void triggerError(Status st) {
    X2JConvertProgress::UpdateQueue q;
    q.fStatus = st;
    q.fPhase = X2JConvertProgress::Phase::Error;
    q.fTotal = 1;
    q.fDone = 1;
    fUpdater->trigger(q);
  }

private:
  File const fInput;
  File const fOutput;
  je2be::box360::Options fOptions;
  std::shared_ptr<AsyncHandler<X2JConvertProgress::UpdateQueue>> fUpdater;
};

X2JConvertProgress::X2JConvertProgress(X2JConfigState const &configState) : fConfigState(configState) {
  auto width = kWindowWidth;
  auto height = kWindowHeight;
  setSize(width, height);

  fConversionProgress = -1;

  {
    fCancelButton.reset(new component::TextButton(TRANS("Cancel")));
    fCancelButton->setBounds(kMargin, height - kMargin - kButtonBaseHeight, kButtonMinWidth, kButtonBaseHeight);
    fCancelButton->onClick = [this]() { onCancelButtonClicked(); };
    addAndMakeVisible(*fCancelButton);
  }

  int y = kMargin;
  {
    fLabel.reset(new Label("", TRANS("Converting...")));
    fLabel->setBounds(kMargin, y, width - 2 * kMargin, kButtonBaseHeight);
    fLabel->setJustificationType(Justification::topLeft);
    addAndMakeVisible(*fLabel);
  }
  int errorMessageY = y + fLabel->getHeight();
  y += fLabel->getHeight() + kMargin;

  {
    fConversionProgressBar.reset(new ProgressBar(fConversionProgress));
    fConversionProgressBar->setBounds(kMargin, y, width - 2 * kMargin, kButtonBaseHeight);
    fConversionProgressBar->setTextToDisplay("Conversion: ");
    addAndMakeVisible(*fConversionProgressBar);
  }

  {
    fErrorMessage.reset(new TextEditor());
    fErrorMessage->setBounds(kMargin, errorMessageY, width - 2 * kMargin, fCancelButton->getY() - kMargin - errorMessageY);
    fErrorMessage->setEnabled(false);
    fErrorMessage->setMultiLine(true);
    fErrorMessage->setColour(TextEditor::backgroundColourId, findColour(Label::backgroundColourId));
    addChildComponent(*fErrorMessage);
  }

  fTaskbarProgress.reset(new TaskbarProgress());

  File temp = TemporaryDirectory::EnsureExisting();
  juce::Uuid u;
  File outputDir = temp.getChildFile(u.toDashedString());
  outputDir.createDirectory();
  fOutputDirectory = outputDir;

  fUpdater = std::make_shared<AsyncHandler<UpdateQueue>>([this](UpdateQueue q) {
    onProgressUpdate(q.fPhase, q.fDone, q.fTotal, q.fStatus);
  });

  je2be::box360::Options opt;
  opt.fTempDirectory = PathFromFile(temp);
  if (fConfigState.fLocalPlayer) {
    juce::Uuid juceUuid = *fConfigState.fLocalPlayer;
    uint8_t data[16];
    std::copy_n(juceUuid.getRawData(), 16, data);
    auto uuid = je2be::Uuid::FromData(data);
    opt.fLocalPlayer = uuid;
  }
  fThread.reset(new X2JWorkerThread(configState.fInputState.fInput, outputDir, opt, fUpdater));
  fThread->startThread();
}

X2JConvertProgress::~X2JConvertProgress() {
  fThread->stopThread(-1);
  fTaskbarProgress->setState(TaskbarProgress::State::NoProgress);
}

void X2JConvertProgress::paint(juce::Graphics &g) {}

void X2JConvertProgress::onCancelButtonClicked() {
  if (fFailed) {
    JUCEApplication::getInstance()->invoke(commands::toChooseXbox360InputToJava, true);
  } else {
    fCancelButton->setEnabled(false);
    fCommandWhenFinished = commands::toXbox360ToJavaConfig;
    fThread->signalThreadShouldExit();
    fConversionProgress = -1;
    fCancelRequested = true;
    fConversionProgressBar->setTextToDisplay({});
    fLabel->setText(TRANS("Waiting for the worker thread to finish"), dontSendNotification);
  }
}

void X2JConvertProgress::onProgressUpdate(Phase phase, double done, double total, Status status) {
  double const weightUnzip = 0.5;
  double const weightConversion = 1 - weightUnzip;
  fFailed = !status.ok();

  if (phase == Phase::Conversion && !fCancelRequested) {
    fLabel->setText(TRANS("Converting..."), dontSendNotification);
    double progress = done / total;
    if (progress > 0) {
      fConversionProgress = progress;
    }
    fTaskbarProgress->setState(TaskbarProgress::State::Normal);
    fTaskbarProgress->update(weightUnzip + progress * weightConversion);
  } else if (phase == Phase::Done && !fCancelRequested) {
    fState = JavaConvertedState(fConfigState.fInputState.fWorldName, fOutputDirectory);
    if (fCommandWhenFinished != commands::toChooseJavaOutput && fOutputDirectory.exists()) {
      TemporaryDirectory::QueueDeletingDirectory(fOutputDirectory);
    }
    if (status.ok()) {
      JUCEApplication::getInstance()->invoke(fCommandWhenFinished, true);
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
    auto error = status.error();
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
    fConversionProgressBar->setVisible(false);
  }
}

} // namespace je2be::desktop::component::x2j
