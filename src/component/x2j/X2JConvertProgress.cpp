#include <je2be.hpp>

#include "CommandID.h"
#include "Constants.h"
#include "File.h"
#include "TaskbarProgress.h"
#include "TemporaryDirectory.h"
#include "component/TextButton.h"
#include "component/x2j/X2JConvertProgress.h"

using namespace juce;

namespace je2be::gui::component::x2j {

class X2JConvertProgress::Updater : public AsyncUpdater {
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

  std::atomic<X2JConvertProgress *> fTarget;
  bool fOk = false;

private:
  std::deque<Entry> fQueue;
  std::mutex fMut;
};

class X2JWorkerThread : public Thread, public je2be::box360::Progress {
public:
  X2JWorkerThread(File input, File output, je2be::box360::Options opt,
                  std::shared_ptr<X2JConvertProgress::Updater> updater)
      : Thread("je2be::gui::X2JConvert"), fInput(input), fOutput(output), fOptions(opt), fUpdater(updater) {}

  void run() override {
    try {
      unsafeRun();
    } catch (std::filesystem::filesystem_error &e) {
      fUpdater->trigger(X2JConvertProgress::Phase::Error, 1, 1);
    } catch (...) {
      fUpdater->trigger(X2JConvertProgress::Phase::Error, 1, 1);
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
    {
      bool ok = je2be::box360::Converter::Run(PathFromFile(fInput), PathFromFile(fOutput), std::thread::hardware_concurrency(), fOptions, this);
      fUpdater->complete(ok);
    }
    fUpdater->trigger(X2JConvertProgress::Phase::Done, 1, 1);
  }

  bool report(double done, double total) override {
    fUpdater->trigger(X2JConvertProgress::Phase::Conversion, done / total, 1.0);
    return !threadShouldExit();
  }

private:
  File const fInput;
  File const fOutput;
  je2be::box360::Options fOptions;
  std::shared_ptr<X2JConvertProgress::Updater> fUpdater;
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
    y += fLabel->getHeight() + kMargin;
  }
  int errorMessageY = y;

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
  fOutputDirectory = outputDir;

  fUpdater = std::make_shared<Updater>();
  fUpdater->fTarget.store(this);

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
    JUCEApplication::getInstance()->invoke(gui::toChooseXbox360InputToJava, true);
  } else {
    fCancelButton->setEnabled(false);
    fCommandWhenFinished = gui::toXbox360ToJavaConfig;
    fThread->signalThreadShouldExit();
    fConversionProgress = -1;
    fCancelRequested = true;
    fConversionProgressBar->setTextToDisplay({});
    fLabel->setText(TRANS("Waiting for the worker thread to finish"), dontSendNotification);
  }
}

void X2JConvertProgress::onProgressUpdate(Phase phase, double done, double total) {
  double const weightUnzip = 0.5;
  double const weightConversion = 1 - weightUnzip;

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
    if (fCommandWhenFinished != gui::toChooseJavaOutput && fOutputDirectory.exists()) {
      TemporaryDirectory::QueueDeletingDirectory(fOutputDirectory);
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
    fConversionProgressBar->setVisible(false);
  }
}

} // namespace je2be::gui::component::x2j
