#include <je2be.hpp>

#include "B2JConvertProgressComponent.h"
#include "CommandID.h"
#include "Constants.h"
#include "File.h"
#include "TaskbarProgress.h"
#include "TemporaryDirectory.h"

using namespace juce;

namespace je2be::gui::b2j {

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
  B2JWorkerThread(File input, je2be::toje::InputOption io, File output, je2be::toje::OutputOption oo,
                  std::shared_ptr<B2JConvertProgressComponent::Updater> updater)
      : Thread("je2be::gui::B2JConvert"), fInput(input), fInputOption(io),
        fOutput(output), fOutputOption(oo), fUpdater(updater) {}

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
    File sessionTempDir = TemporaryDirectory::EnsureExisting();
    juce::Uuid u;
    File temp = sessionTempDir.getChildFile(u.toDashedString());
    temp.createDirectory();
    defer {
      TemporaryDirectory::QueueDeletingDirectory(temp);
    };

    File input;
    if (fInput.isDirectory()) {
      if (!copyInto(temp)) {
        fUpdater->trigger(B2JConvertProgressComponent::Phase::Error, 1, 1);
        return;
      }
      input = temp;
      fUpdater->trigger(B2JConvertProgressComponent::Phase::Unzip, 1, 1);
    } else {
      if (!unzipInto(temp)) {
        fUpdater->trigger(B2JConvertProgressComponent::Phase::Error, 1, 1);
        return;
      }
      input = temp;
    }
    {
      je2be::toje::Converter c(PathFromFile(input), fInputOption, PathFromFile(fOutput), fOutputOption);
      bool ok = c.run(std::thread::hardware_concurrency(), this);
      fUpdater->complete(ok);
    }
    fUpdater->trigger(B2JConvertProgressComponent::Phase::Done, 1, 1);
  }

  bool copyInto(File temp) {
    using namespace leveldb;

    auto env = leveldb::Env::Default();
    File db = fInput.getChildFile("db");

    if (!temp.getChildFile("db").createDirectory()) {
      return false;
    }

    std::vector<File> exclude;

    File lockFile = db.getChildFile("LOCK");
    auto lockFilePath = PathFromFile(lockFile);
    FileLock *lock = nullptr;
    if (auto st = env->LockFile(lockFilePath, &lock); !st.ok()) {
      return false;
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
      String content = currentFile.loadFileAsString();
      String manifestName = content.trimEnd();
      File manifestFile = db.getChildFile(manifestName);
      if (manifestFile.existsAsFile()) {

        // This will success even when the game client is opening the db.
        if (!manifestFile.copyFileTo(temp.getChildFile("db").getChildFile(manifestName))) {
          return false;
        }

        if (auto st = env->LockFile(PathFromFile(manifestFile), &manifestLock); !st.ok()) {
          return false;
        }
        exclude.push_back(manifestFile);
      }
    }
    defer {
      if (manifestLock) {
        env->UnlockFile(manifestLock);
      }
    };

    if (!CopyDirectoryRecursive(fInput, temp, exclude)) {
      return false;
    }

    return true;
  }

  bool unzipInto(File temp) {
    ZipFile zip(fInput);
    int numEntries = zip.getNumEntries();
    for (int i = 0; i < numEntries; ++i) {
      auto result = zip.uncompressEntry(i, temp);
      if (result.failed()) {
        return false;
      }
      fUpdater->trigger(B2JConvertProgressComponent::Phase::Unzip, i + 1, numEntries);
    }
    return true;
  }

  bool report(double done, double total) override {
    fUpdater->trigger(B2JConvertProgressComponent::Phase::Conversion, done / total, 1.0);
    return !threadShouldExit();
  }

private:
  File const fInput;
  je2be::toje::InputOption fInputOption;
  File const fOutput;
  je2be::toje::OutputOption fOutputOption;
  std::shared_ptr<B2JConvertProgressComponent::Updater> fUpdater;
};

B2JConvertProgressComponent::B2JConvertProgressComponent(B2JConfigState const &configState) : fState(configState) {
  auto width = kWindowWidth;
  auto height = kWindowHeight;
  setSize(width, height);

  fUnzipOrCopyProgress = -1;
  fConversionProgress = 0;

  {
    fCancelButton.reset(new TextButton(TRANS("Cancel")));
    fCancelButton->setBounds(kMargin, height - kMargin - kButtonBaseHeight, kButtonMinWidth, kButtonBaseHeight);
    fCancelButton->setMouseCursor(MouseCursor::PointingHandCursor);
    fCancelButton->onClick = [this]() { onCancelButtonClicked(); };
    addAndMakeVisible(*fCancelButton);
  }

  bool needsUnzip = !configState.fInputState.fInputFileOrDirectory->isDirectory();

  int y = kMargin;
  {
    String title;
    if (needsUnzip) {
      title = TRANS("Unzipping...");
    } else {
      title = TRANS("Copying...");
    }
    fLabel.reset(new Label("", title));
    fLabel->setBounds(kMargin, y, width - 2 * kMargin, kButtonBaseHeight);
    fLabel->setJustificationType(Justification::topLeft);
    addAndMakeVisible(*fLabel);
    y += fLabel->getHeight() + kMargin;
  }
  int errorMessageY = y;

  {
    String title;
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
    fErrorMessage->setBounds(kMargin, errorMessageY, width - 2 * kMargin, fCancelButton->getY() - y - kMargin);
    fErrorMessage->setEnabled(false);
    fErrorMessage->setMultiLine(true);
    addChildComponent(*fErrorMessage);
  }

  fTaskbarProgress.reset(new TaskbarProgress());

  File temp = TemporaryDirectory::EnsureExisting();
  juce::Uuid u;
  File outputDir = temp.getChildFile(u.toDashedString());
  outputDir.createDirectory();
  fState.fOutputDirectory = outputDir;

  fUpdater = std::make_shared<Updater>();
  fUpdater->fTarget.store(this);

  je2be::toje::InputOption io;
  if (fState.fConfigState.fLocalPlayer) {
    juce::Uuid juceUuid = *fState.fConfigState.fLocalPlayer;
    uint8_t data[16];
    std::copy_n(juceUuid.getRawData(), 16, data);
    auto uuid = je2be::Uuid::FromData(data);
    io.fLocalPlayer = uuid;
  }
  je2be::toje::OutputOption oo;
  fThread.reset(new B2JWorkerThread(*configState.fInputState.fInputFileOrDirectory, io, fState.fOutputDirectory, oo, fUpdater));
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
    if (fCommandWhenFinished != gui::toB2JChooseOutput && fState.fOutputDirectory.exists()) {
      TemporaryDirectory::QueueDeletingDirectory(fState.fOutputDirectory);
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
    fCancelButton->setMouseCursor(MouseCursor::PointingHandCursor);
    fCancelButton->setEnabled(true);
    fUnzipOrCopyProgressBar->setVisible(false);
    fConversionProgressBar->setVisible(false);
  }
}

} // namespace je2be::gui::b2j
