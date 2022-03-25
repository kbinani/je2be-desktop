#include "component/CopyBedrockArtifactProgress.h"
#include "CommandID.h"
#include "ComponentState.h"
#include "Constants.h"
#include "TaskbarProgress.h"
#include "TemporaryDirectory.h"

using namespace juce;

namespace je2be::gui::component {

class J2BCopyThread : public CopyBedrockArtifactProgress::Worker {
public:
  J2BCopyThread(AsyncUpdater *updater, File from, File to, double *progress)
      : CopyBedrockArtifactProgress::Worker("j2b::gui::J2BCopyThread"),
        fUpdater(updater), fFrom(from), fTo(to), fProgress(progress) {}

  void run() override {
    try {
      unsafeRun();
    } catch (...) {
      fResult = CopyBedrockArtifactProgress::Worker::Result::Failed;
    }
    fUpdater->triggerAsyncUpdate();
  }

  std::optional<CopyBedrockArtifactProgress::Worker::Result> result() const override {
    return fResult;
  }

private:
  void unsafeRun() {
    RangedDirectoryIterator it(fFrom, true);
    for (auto const &item : it) {
      File const &from = item.getFile();
      if (from.isDirectory()) {
        continue;
      }
      auto relative = from.getRelativePathFrom(fFrom);
      auto destination = fTo.getChildFile(relative);
      auto dir = destination.getParentDirectory();
      if (!dir.exists()) {
        auto result = dir.createDirectory();
        if (!result.wasOk()) {
          fResult = CopyBedrockArtifactProgress::Worker::Result::Failed;
          return;
        }
      }
      if (!from.copyFileTo(destination)) {
        fResult = CopyBedrockArtifactProgress::Worker::Result::Failed;
        return;
      }
      *fProgress = (double)item.getEstimatedProgress();
    }
    fResult = CopyBedrockArtifactProgress::Worker::Result::Success;
  }

private:
  AsyncUpdater *const fUpdater;
  File fFrom;
  File fTo;
  std::optional<CopyBedrockArtifactProgress::Worker::Result> fResult;
  double *const fProgress;
};

class ZipThread : public CopyBedrockArtifactProgress::Worker {
public:
  ZipThread(AsyncUpdater *updater, File from, File to, double *progress)
      : CopyBedrockArtifactProgress::Worker("j2b::gui::ZipThread"), fUpdater(updater),
        fFrom(from), fTo(to), fProgress(progress) {}

  void run() override {
    try {
      unsafeRun();
    } catch (...) {
      fResult = CopyBedrockArtifactProgress::Worker::Result::Failed;
    }
    fUpdater->triggerAsyncUpdate();
  }

  std::optional<CopyBedrockArtifactProgress::Worker::Result> result() const override {
    return fResult;
  }

private:
  void unsafeRun() {
    ZipFile::Builder builder;
    RangedDirectoryIterator it(fFrom, true);
    int const kCompressionLevel = 9;
    bool cancelled = false;
    for (auto const &item : it) {
      File file = item.getFile();
      String relativePath = file.getRelativePathFrom(fFrom);
      builder.addFile(file, kCompressionLevel, relativePath);
      if (currentThreadShouldExit()) {
        cancelled = true;
        break;
      }
    }

    if (cancelled || currentThreadShouldExit()) {
      fResult = CopyBedrockArtifactProgress::Worker::Result::Cancelled;
      return;
    }
    fResult = CopyBedrockArtifactProgress::Worker::Result::Failed;
    auto stream = fTo.createOutputStream();
    if (!stream) {
      return;
    }
    if (!stream->setPosition(0)) {
      return;
    }
    if (auto result = stream->truncate(); !result.ok()) {
      return;
    }
    if (!builder.writeToStream(*stream, fProgress)) {
      return;
    }
    fResult = CopyBedrockArtifactProgress::Worker::Result::Success;
  }

private:
  AsyncUpdater *const fUpdater;
  File fFrom;
  File fTo;
  std::optional<CopyBedrockArtifactProgress::Worker::Result> fResult;
  double *const fProgress;
};

CopyBedrockArtifactProgress::CopyBedrockArtifactProgress(J2BChooseOutputState const &state) : fState(state) {
  auto width = kWindowWidth;
  auto height = kWindowHeight;
  setSize(width, height);

  fLabel.reset(new Label("", TRANS("Saving...")));
  fLabel->setBounds(kMargin, kMargin, width - 2 * kMargin, kButtonBaseHeight);
  fLabel->setJustificationType(Justification::topLeft);
  addAndMakeVisible(*fLabel);

  fProgressBar.reset(new ProgressBar(fProgress));
  fProgressBar->setBounds(kMargin, kMargin + kButtonBaseHeight + kMargin, width - 2 * kMargin, kButtonBaseHeight);
  addAndMakeVisible(*fProgressBar);

  fTaskbarProgress.reset(new TaskbarProgress());

  if (state.fFormat == J2BOutputFormat::Directory) {
    fCopyThread.reset(new J2BCopyThread(this, state.fConvertState.fOutputDirectory, *state.fCopyDestination, &fProgress));
  } else {
    fCopyThread.reset(new ZipThread(this, state.fConvertState.fOutputDirectory, *state.fCopyDestination, &fProgress));
  }
  fCopyThread->startThread();
  fTaskbarProgress->setState(TaskbarProgress::State::Normal);
  startTimerHz(12);
}

CopyBedrockArtifactProgress::~CopyBedrockArtifactProgress() {
  fTaskbarProgress->setState(TaskbarProgress::State::NoProgress);
}

void CopyBedrockArtifactProgress::paint(juce::Graphics &g) {}

void CopyBedrockArtifactProgress::handleAsyncUpdate() {
  struct InvokeToChooseOutput : public ModalComponentManager::Callback {
    void modalStateFinished(int returnValue) override {
      JUCEApplication::getInstance()->invoke(gui::toChooseBedrockOutput, true);
    }
  };

  struct InvokeToModeSelect : public ModalComponentManager::Callback {
    void modalStateFinished(int returnValue) override {
      JUCEApplication::getInstance()->invoke(gui::toSelectInputType, true);
    }
  };

  stopTimer();

  auto result = fCopyThread->result();
  if (!result || *result == CopyBedrockArtifactProgress::Worker::Result::Failed) {
    fTaskbarProgress->setState(TaskbarProgress::State::Error);
    NativeMessageBox::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon, TRANS("Failed"), TRANS("Saving failed."), nullptr, new InvokeToChooseOutput);
  } else if (*result == CopyBedrockArtifactProgress::Worker::Result::Cancelled) {
    fTaskbarProgress->setState(TaskbarProgress::State::NoProgress);
    NativeMessageBox::showMessageBoxAsync(AlertWindow::AlertIconType::InfoIcon, TRANS("Cancelled"), TRANS("Saving cancelled."), nullptr, new InvokeToChooseOutput);
    JUCEApplication::getInstance()->invoke(gui::toChooseBedrockOutput, true);
  } else {
    fTaskbarProgress->setState(TaskbarProgress::State::NoProgress);
    NativeMessageBox::showMessageBoxAsync(AlertWindow::AlertIconType::InfoIcon, TRANS("Completed"), TRANS("Saving completed.") + "\n" + fState.fCopyDestination->getFullPathName(), nullptr, new InvokeToModeSelect);
    if (fState.fConvertState.fOutputDirectory.exists()) {
      TemporaryDirectory::QueueDeletingDirectory(fState.fConvertState.fOutputDirectory);
    }
  }
}

void CopyBedrockArtifactProgress::timerCallback() {
  double progress = fProgress;
  fTaskbarProgress->update(progress);
}

} // namespace je2be::gui::component
