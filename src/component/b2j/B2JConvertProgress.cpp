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

    File input;
    if (fInput.isDirectory()) {
      triggerProgress(B2JConvertProgress::Phase::Unzip, -1, 0);
      if (auto st = copyInto(temp); !st.ok()) {
        triggerError(st);
        return;
      }
      input = temp;
      triggerProgress(B2JConvertProgress::Phase::Unzip, 1, 0);
    } else {
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

  je2be::Status copyInto(File temp) {
    using namespace leveldb;

    auto env = leveldb::Env::Default();
    File db = fInput.getChildFile("db");

    if (auto st = temp.getChildFile("db").createDirectory(); !st.ok()) {
      return Error(__FILE__, __LINE__, st.getErrorMessage().toStdString());
    }

    std::vector<File> exclude;

    File lockFile = db.getChildFile("LOCK");
    auto lockFilePath = PathFromFile(lockFile);
    FileLock *lock = nullptr;
    if (auto st = env->LockFile(lockFilePath, &lock); !st.ok()) {
      return Error(__FILE__, __LINE__, st.ToString());
    }
    exclude.push_back(lockFile);
    defer {
      env->UnlockFile(lock);
    };

    // Bedrock game client doesn't create or lock the "LOCK" file.
    // The locking process above is for other app reading the db in regular manner.
    // For bedrock game client, additionally lock the manifest file.
    FileLock *manifestLock = nullptr;
    File currentFile = db.getChildFile("CURRENT");
    if (currentFile.existsAsFile()) {
      juce::String content = currentFile.loadFileAsString();
      juce::String manifestName = content.trimEnd();
      File manifestFile = db.getChildFile(manifestName);
      if (manifestFile.existsAsFile()) {

        // This will success even when the game client is opening the db.
        if (auto st = CopyFile(manifestFile, temp.getChildFile("db").getChildFile(manifestName), __FILE__, __LINE__); !st.ok()) {
          return st;
        }

        if (auto st = env->LockFile(PathFromFile(manifestFile), &manifestLock); !st.ok()) {
          return Error(__FILE__, __LINE__, st.ToString());
        }
        exclude.push_back(manifestFile);
      }
    }
    defer {
      if (manifestLock) {
        env->UnlockFile(manifestLock);
      }
    };

    if (auto st = CopyDirectoryRecursive(fInput, temp, exclude); !st.ok()) {
      return st;
    }

    return Status::Ok();
  }

  je2be::Status unzipInto(File temp) {
    juce::ZipFile zip(fInput);
    double numEntries = zip.getNumEntries();
    for (int i = 0; i < numEntries; ++i) {
      auto result = zip.uncompressEntry(i, temp);
      if (result.failed()) {
        return Error(__FILE__, __LINE__, result.getErrorMessage().toStdString());
      }
      triggerProgress(B2JConvertProgress::Phase::Unzip, (i + 1) / numEntries, 0);
    }
    return Status::Ok();
  }

  bool reportConvert(double progress, uint64_t numConvertedChunks) override {
    triggerProgress(B2JConvertProgress::Phase::Conversion, progress, numConvertedChunks);
    return !threadShouldExit();
  }

  bool reportTerraform(double progress, uint64_t numProcessedChunks) override {
    triggerProgress(B2JConvertProgress::Phase::PostProcess, progress, numProcessedChunks);
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

  {
    juce::String title;
    if (needsUnzip) {
      title = "Unzip: ";
    } else {
      title = "Copy: ";
    }
    fUnzipOrCopyProgressBar.reset(new ProgressBar(fUnzipOrCopyProgress));
    fUnzipOrCopyProgressBar->setBounds(kMargin, y, width - 2 * kMargin, kButtonBaseHeight);
    fUnzipOrCopyProgressBar->setTextToDisplay(title);
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
    fConversionProgress = -1;
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
    fUnzipOrCopyProgress = progress;
    if (progress >= 1) {
      fLabel->setText(TRANS("Converting..."), dontSendNotification);
      fConversionProgress = -1;
    } else {
      fConversionProgress = 0;
    }
    fTaskbarProgress->setState(TaskbarProgress::State::Normal);
    fTaskbarProgress->update(progress * weightUnzip);
  } else if (phase == Phase::Conversion && !fCancelRequested) {
    fLabel->setText(TRANS("Converting..."), dontSendNotification);
    if (progress > 0) {
      fConversionProgress = progress;
    }
    fUnzipOrCopyProgress = 1;
    fTaskbarProgress->setState(TaskbarProgress::State::Normal);
    fTaskbarProgress->update(weightUnzip + progress * weightConversion);
    fConversionProgressBar->setTextToDisplay("Converted " + juce::String(numConvertedChunks) + " Chunks: ");
  } else if (phase == Phase::PostProcess && !fCancelRequested) {
    fLabel->setText(TRANS("Post processing..."), dontSendNotification);
    if (progress > 0) {
      fPostProcessProgress = progress;
    }
    fConversionProgress = 1;
    fTaskbarProgress->setState(TaskbarProgress::State::Normal);
    fTaskbarProgress->update(weightUnzip + weightConversion + progress * weightPostProcess);
    fPostProcessProgressBar->setTextToDisplay("Post processed " + juce::String(numConvertedChunks) + " Chunks: ");
  } else if (phase == Phase::Done) {
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
      message += juce::String("\nFailed: where: ") + error->fWhere.fFile + ":" + std::to_string(error->fWhere.fLine);
      if (!error->fWhat.empty()) {
        message += juce::String(", what: " + error->fWhat);
      }
      fErrorMessage->setText(message);
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
