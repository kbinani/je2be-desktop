#include <je2be.hpp>

#include "CommandID.h"
#include "Constants.h"
#include "File.h"
#include "TaskbarProgress.h"
#include "TemporaryDirectory.h"
#include "component/TextButton.h"
#include "component/b2j/B2JConvertProgress.h"

using namespace juce;

namespace je2be::desktop::component::b2j {

class B2JConvertProgress::Updater : public AsyncUpdater {
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
      target->onProgressUpdate(e.fPhase, e.fDone, e.fTotal, fStatus);
    }
  }

  void complete(je2be::Status status) {
    fStatus = status;
  }

  std::atomic<B2JConvertProgress *> fTarget;

private:
  std::deque<Entry> fQueue;
  std::mutex fMut;
  je2be::Status fStatus;
};

class B2JWorkerThread : public Thread, public je2be::toje::Progress {
public:
  B2JWorkerThread(File input, File output, je2be::toje::Options opt,
                  std::shared_ptr<B2JConvertProgress::Updater> updater)
      : Thread("je2be::desktop::component::b2j::B2JWorkerThread"), fInput(input), fOutput(output), fOptions(opt), fUpdater(updater) {}

  void run() override {
    try {
      unsafeRun();
    } catch (std::filesystem::filesystem_error &e) {
      fUpdater->complete(Error(__FILE__, __LINE__, e.what()));
      fUpdater->trigger(B2JConvertProgress::Phase::Error, 1, 1);
    } catch (std::exception &e) {
      fUpdater->complete(Error(__FILE__, __LINE__, e.what()));
      fUpdater->trigger(B2JConvertProgress::Phase::Error, 1, 1);
    } catch (char const *what) {
      fUpdater->complete(Error(__FILE__, __LINE__, what));
      fUpdater->trigger(B2JConvertProgress::Phase::Error, 1, 1);
    } catch (...) {
      fUpdater->complete(Error(__FILE__, __LINE__));
      fUpdater->trigger(B2JConvertProgress::Phase::Error, 1, 1);
    }
  }

  void unsafeRun() {
    File sessionTempDir = TemporaryDirectory::EnsureExisting();
    juce::Uuid u;
    File temp = sessionTempDir.getChildFile(u.toDashedString());
    if (auto st = temp.createDirectory(); !st.ok()) {
      fUpdater->complete(Error(__FILE__, __LINE__, st.getErrorMessage().toStdString()));
      fUpdater->trigger(B2JConvertProgress::Phase::Error, 1, 1);
      return;
    }
    defer {
      TemporaryDirectory::QueueDeletingDirectory(temp);
    };

    File input;
    if (fInput.isDirectory()) {
      if (auto st = copyInto(temp); !st.ok()) {
        fUpdater->complete(st);
        fUpdater->trigger(B2JConvertProgress::Phase::Error, 1, 1);
        return;
      }
      input = temp;
      fUpdater->trigger(B2JConvertProgress::Phase::Unzip, 1, 1);
    } else {
      if (auto st = unzipInto(temp); !st.ok()) {
        fUpdater->complete(st);
        fUpdater->trigger(B2JConvertProgress::Phase::Error, 1, 1);
        return;
      }
      input = temp;
    }
    {
      je2be::toje::Converter c(PathFromFile(input), PathFromFile(fOutput), fOptions);
      auto st = c.run(std::thread::hardware_concurrency(), this);
      fUpdater->complete(st);
    }
    fUpdater->trigger(B2JConvertProgress::Phase::Done, 1, 1);
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
        if (!manifestFile.copyFileTo(temp.getChildFile("db").getChildFile(manifestName))) {
          return Error(__FILE__, __LINE__);
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
    int numEntries = zip.getNumEntries();
    for (int i = 0; i < numEntries; ++i) {
      auto result = zip.uncompressEntry(i, temp);
      if (result.failed()) {
        return Error(__FILE__, __LINE__, result.getErrorMessage().toStdString());
      }
      fUpdater->trigger(B2JConvertProgress::Phase::Unzip, i + 1, numEntries);
    }
    return Status::Ok();
  }

  bool report(double done, double total) override {
    fUpdater->trigger(B2JConvertProgress::Phase::Conversion, done / total, 1.0);
    return !threadShouldExit();
  }

private:
  File const fInput;
  File const fOutput;
  je2be::toje::Options fOptions;
  std::shared_ptr<B2JConvertProgress::Updater> fUpdater;
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

  fUpdater = std::make_shared<Updater>();
  fUpdater->fTarget.store(this);

  je2be::toje::Options opt;
  opt.fTempDirectory = PathFromFile(temp);
  if (fConfigState.fLocalPlayer) {
    juce::Uuid juceUuid = *fConfigState.fLocalPlayer;
    uint8_t data[16];
    std::copy_n(juceUuid.getRawData(), 16, data);
    auto uuid = je2be::Uuid::FromData(data);
    opt.fLocalPlayer = uuid;
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

void B2JConvertProgress::onProgressUpdate(Phase phase, double done, double total, Status st) {
  double const weightUnzip = 0.5;
  double const weightConversion = 1 - weightUnzip;
  fFailed = !st.ok();

  if (phase == Phase::Unzip && !fCancelRequested) {
    double progress = done / total;
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
    double progress = done / total;
    if (progress > 0) {
      fConversionProgress = progress;
    }
    fUnzipOrCopyProgress = 1;
    fTaskbarProgress->setState(TaskbarProgress::State::Normal);
    fTaskbarProgress->update(weightUnzip + progress * weightConversion);
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
  }
}

} // namespace je2be::desktop::component::b2j
