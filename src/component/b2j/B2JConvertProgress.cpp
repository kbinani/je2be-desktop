#include <je2be.hpp>

#include <leveldb/env.h>

#include "CommandID.h"
#include "Constants.h"
#include "File.h"
#include "TaskbarProgress.h"
#include "TemporaryDirectory.h"
#include "component/TextButton.h"
#include "component/b2j/B2JConvertProgress.h"

using namespace juce;

namespace je2be::desktop::component::b2j {

class B2JWorkerThread : public Thread, public je2be::toje::Progress {
public:
  B2JWorkerThread(File input, File output, je2be::toje::Options opt,
                  std::shared_ptr<AsyncHandler<B2JConvertProgress::UpdateQueue>> updater)
      : Thread("je2be::desktop::component::b2j::B2JWorkerThread"), fInput(input), fOutput(output), fOptions(opt), fUpdater(updater) {}

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
    File temp;
    defer {
      if (temp != File()) {
        TemporaryDirectory::QueueDeletingDirectory(temp);
      }
    };

    File input;
    if (fInput.isDirectory()) {
      input = fInput;
      triggerProgress(B2JConvertProgress::Phase::Unzip, 1, 0);
    } else {
      File sessionTempDir = TemporaryDirectory::EnsureExisting();
      juce::Uuid u;
      temp = sessionTempDir.getChildFile(u.toDashedString());
      if (auto st = temp.createDirectory(); !st.ok()) {
        triggerError(Error(__FILE__, __LINE__, st.getErrorMessage().toStdString()));
        return;
      }
      if (auto st = unzipInto(temp); !st.ok()) {
        triggerError(st);
        return;
      }
      input = temp;
    }
    {
      auto st = je2be::toje::Converter::Run(PathFromFile(input), PathFromFile(fOutput), fOptions, std::thread::hardware_concurrency(), this);
      triggerProgress(B2JConvertProgress::Phase::Done, 1, 0, st);
    }
  }

  je2be::Status unzipInto(File temp) {
    juce::ZipFile file(fInput);
    int const total = file.getNumEntries();
    for (int i = 0; i < total; i++) {
      auto ret = file.uncompressEntry(i, temp);
      if (!ret.ok()) {
        return Error(__FILE__, __LINE__, ret.getErrorMessage().toStdString());
      }
      triggerProgress(B2JConvertProgress::Phase::Unzip, (i + 1) / (double)total, 0);
    }
    return je2be::Status::Ok();
  }

  bool reportConvert(Rational<u64> const &progress, uint64_t numConvertedChunks) override {
    triggerProgress(B2JConvertProgress::Phase::Conversion, progress.toD(), numConvertedChunks);
    return !threadShouldExit();
  }

  bool reportTerraform(Rational<u64> const &progress, uint64_t numProcessedChunks) override {
    triggerProgress(B2JConvertProgress::Phase::PostProcess, progress.toD(), numProcessedChunks);
    return !threadShouldExit();
  }

  void triggerProgress(B2JConvertProgress::Phase phase, double progress, uint64_t numConvertedChunks, Status st = Status::Ok()) {
    B2JConvertProgress::UpdateQueue q;
    q.fStatus = st;
    q.fPhase = phase;
    q.fProgress = progress;
    q.fNumConvertedChunks = numConvertedChunks;
    fUpdater->trigger(q);
  }

  void triggerError(Status st) {
    B2JConvertProgress::UpdateQueue q;
    q.fStatus = st;
    q.fPhase = B2JConvertProgress::Phase::Error;
    q.fProgress = 1;
    q.fNumConvertedChunks = 0;
    fUpdater->trigger(q);
  }

private:
  File const fInput;
  File const fOutput;
  je2be::toje::Options fOptions;
  std::shared_ptr<AsyncHandler<B2JConvertProgress::UpdateQueue>> fUpdater;
};

B2JConvertProgress::B2JConvertProgress(B2JConfigState const &configState) : fConfigState(configState) {
  auto width = kWindowWidth;
  auto height = kWindowHeight;
  setSize(width, height);

  fUnzipOrCopyProgress = -1;
  fConversionProgress = 0;

  {
    fCancelButton.reset(new component::TextButton(TRANS("Cancel")));
    fCancelButton->setBounds(kMargin, height - kMargin - kButtonBaseHeight, kButtonMinWidth, kButtonBaseHeight);
    fCancelButton->onClick = [this]() { onCancelButtonClicked(); };
    addAndMakeVisible(*fCancelButton);
  }

  bool needsUnzip = !configState.fInputState.fInput.isDirectory();

  int y = kMargin;
  {
    juce::String title;
    if (needsUnzip) {
      title = TRANS("Unzipping...");
    } else {
      title = TRANS("Copying...");
    }
    fLabel.reset(new Label("", title));
    fLabel->setBounds(kMargin, y, width - 2 * kMargin, kButtonBaseHeight);
    fLabel->setJustificationType(Justification::topLeft);
    addAndMakeVisible(*fLabel);
  }
  int errorMessageY = y + fLabel->getHeight();
  y += fLabel->getHeight() + kMargin;

  fUnzipOrCopyProgressBar.reset(new ProgressBar(fUnzipOrCopyProgress));
  if (needsUnzip) {
    fUnzipOrCopyProgressBar->setBounds(kMargin, y, width - 2 * kMargin, kButtonBaseHeight);
    fUnzipOrCopyProgressBar->setTextToDisplay("Unzip: ");
    addAndMakeVisible(*fUnzipOrCopyProgressBar);
    y += fUnzipOrCopyProgressBar->getHeight() + kMargin;
  }

  {
    fConversionProgressBar.reset(new ProgressBar(fConversionProgress));
    fConversionProgressBar->setBounds(kMargin, y, width - 2 * kMargin, kButtonBaseHeight);
    fConversionProgressBar->setTextToDisplay("Conversion: ");
    addAndMakeVisible(*fConversionProgressBar);
    y += fConversionProgressBar->getHeight() + kMargin;
  }

  {
    fPostProcessProgressBar.reset(new ProgressBar(fPostProcessProgress));
    fPostProcessProgressBar->setBounds(kMargin, y, width - 2 * kMargin, kButtonBaseHeight);
    fPostProcessProgressBar->setTextToDisplay("Post process: ");
    addAndMakeVisible(*fPostProcessProgressBar);
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
    onProgressUpdate(q.fPhase, q.fProgress, q.fNumConvertedChunks, q.fStatus);
  });

  je2be::toje::Options opt;
  opt.fTempDirectory = PathFromFile(temp);
  if (fConfigState.fLocalPlayer) {
    juce::Uuid juceUuid = *fConfigState.fLocalPlayer;
    uint8_t data[16];
    std::copy_n(juceUuid.getRawData(), 16, data);
    auto uuid = je2be::Uuid::FromData(data);
    opt.fLocalPlayer = std::make_shared<Uuid>(uuid);
  }
  fThread.reset(new B2JWorkerThread(configState.fInputState.fInput, outputDir, opt, fUpdater));
  fThread->startThread();
}

B2JConvertProgress::~B2JConvertProgress() {
  fThread->stopThread(-1);
  fTaskbarProgress->setState(TaskbarProgress::State::NoProgress);
}

void B2JConvertProgress::paint(juce::Graphics &g) {}

void B2JConvertProgress::onCancelButtonClicked() {
  if (fFailed) {
    JUCEApplication::getInstance()->invoke(commands::toChooseBedrockInput, true);
  } else {
    fCancelButton->setEnabled(false);
    fCommandWhenFinished = commands::toB2JConfig;
    fThread->signalThreadShouldExit();
    if (0 < fUnzipOrCopyProgress && fUnzipOrCopyProgress < 1) {
      fUnzipOrCopyProgress = -1;
      fConversionProgress = 0;
      fPostProcessProgress = 0;
    } else if (0 < fConversionProgress && fConversionProgress < 1) {
      fUnzipOrCopyProgress = 1;
      fConversionProgress = -1;
      fPostProcessProgress = 0;
    } else if (0 < fPostProcessProgress && fPostProcessProgress < 1) {
      fUnzipOrCopyProgress = 1;
      fConversionProgress = 1;
      fPostProcessProgress = -1;
    }
    fCancelRequested = true;
    fLabel->setText(TRANS("Waiting for the worker thread to finish"), dontSendNotification);
  }
}

void B2JConvertProgress::onProgressUpdate(Phase phase, double progress, uint64_t numConvertedChunks, Status st) {
  double const weightUnzip = 0.01;                                     // 0:02.46
  double const weightConversion = 0.31;                                // 2:13.28
  double const weightPostProcess = 1 - weightUnzip - weightConversion; // 4:56.09
  fFailed = !st.ok();

  if (phase == Phase::Unzip && !fCancelRequested) {
    if (progress >= 1) {
      fUnzipOrCopyProgress = 1;
      fConversionProgress = -1;
      fLabel->setText(TRANS("Converting..."), dontSendNotification);
    } else if (progress > 0) {
      fUnzipOrCopyProgress = progress;
      fConversionProgress = 0;
    }
    fPostProcessProgress = 0;
    fTaskbarProgress->setState(TaskbarProgress::State::Normal);
    fTaskbarProgress->update(progress * weightUnzip);
  } else if (phase == Phase::Conversion && !fCancelRequested) {
    fLabel->setText(TRANS("Converting..."), dontSendNotification);
    fUnzipOrCopyProgress = 1;
    if (progress >= 1) {
      fConversionProgress = progress;
      fPostProcessProgress = -1;
      fLabel->setText(TRANS("Post processing..."), dontSendNotification);
    } else if (progress > 0) {
      fConversionProgress = progress;
      fPostProcessProgress = 0;
    }
    fTaskbarProgress->setState(TaskbarProgress::State::Normal);
    fTaskbarProgress->update(weightUnzip + progress * weightConversion);
    fConversionProgressBar->setTextToDisplay("Converted " + juce::String(numConvertedChunks) + " Chunks: ");
  } else if (phase == Phase::PostProcess && !fCancelRequested) {
    fLabel->setText(TRANS("Post processing..."), dontSendNotification);
    fUnzipOrCopyProgress = 1;
    fConversionProgress = 1;
    if (progress >= 1) {
      fPostProcessProgress = 1;
    } else if (progress > 0) {
      fPostProcessProgress = progress;
    }
    fTaskbarProgress->setState(TaskbarProgress::State::Normal);
    fTaskbarProgress->update(weightUnzip + weightConversion + progress * weightPostProcess);
    fPostProcessProgressBar->setTextToDisplay("Post processed " + juce::String(numConvertedChunks) + " Chunks: ");
  } else if (phase == Phase::Done) {
    fUnzipOrCopyProgress = 1;
    fConversionProgress = 1;
    fPostProcessProgress = 1;
    fState = JavaConvertedState(fConfigState.fInputState.fWorldName, fOutputDirectory);
    if (fCommandWhenFinished != commands::toChooseJavaOutput && fOutputDirectory.exists()) {
      TemporaryDirectory::QueueDeletingDirectory(fOutputDirectory);
    }
    if (st.ok()) {
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
    auto error = st.error();
    if (error) {
      juce::String message = juce::String(JUCE_APPLICATION_NAME_STRING) + " version " + JUCE_APPLICATION_VERSION_STRING;
      message += juce::String("\nFailed:\n");
      if (!error->fWhat.empty()) {
        message += juce::String("  what: " + error->fWhat + "\n");
      }
      message += juce::String("  trace: \n");
      for (int i = error->fTrace.size() - 1; i >= 0; i--) {
        auto const &trace = error->fTrace[i];
        message += juce::String("    " + trace.fFile + ":" + std::to_string(trace.fLine) + "\n");
      }
      fErrorMessage->setText(message.trimEnd());
      fErrorMessage->setVisible(true);
    }
    fCancelButton->setButtonText(TRANS("Back"));
    fCancelButton->setEnabled(true);
    fUnzipOrCopyProgressBar->setVisible(false);
    fConversionProgressBar->setVisible(false);
    fPostProcessProgressBar->setVisible(false);
  }
}

} // namespace je2be::desktop::component::b2j
