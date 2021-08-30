#include "CopyProgressComponent.h"
#include "CommandID.h"
#include "ComponentState.h"
#include "Constants.h"

using namespace juce;

class CopyThread : public CopyProgressComponent::Worker {
public:
  CopyThread(AsyncUpdater *updater, File from, File to, double *progress)
      : CopyProgressComponent::Worker("j2b::gui::CopyThread"),
        fUpdater(updater), fFrom(from), fTo(to), fProgress(progress) {}

  void run() override {
    try {
      unsafeRun();
    } catch (...) {
      fResult = CopyProgressComponent::Worker::Result::Failed;
    }
    fUpdater->triggerAsyncUpdate();
  }

  std::optional<CopyProgressComponent::Worker::Result> result() const override {
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
          fResult = CopyProgressComponent::Worker::Result::Failed;
          return;
        }
      }
      if (!from.copyFileTo(destination)) {
        fResult = CopyProgressComponent::Worker::Result::Failed;
        return;
      }
      *fProgress = (double)item.getEstimatedProgress();
    }
    fResult = CopyProgressComponent::Worker::Result::Success;
  }

private:
  AsyncUpdater *const fUpdater;
  File fFrom;
  File fTo;
  std::optional<CopyProgressComponent::Worker::Result> fResult;
  double *const fProgress;
};

class ZipThread : public CopyProgressComponent::Worker {
public:
  ZipThread(AsyncUpdater *updater, File from, File to, double *progress)
      : CopyProgressComponent::Worker("j2b::gui::ZipThread"), fUpdater(updater),
        fFrom(from), fTo(to), fProgress(progress) {}

  void run() override {
    try {
      unsafeRun();
    } catch (...) {
      fResult = CopyProgressComponent::Worker::Result::Failed;
    }
    fUpdater->triggerAsyncUpdate();
  }

  std::optional<CopyProgressComponent::Worker::Result> result() const override {
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
      fResult = CopyProgressComponent::Worker::Result::Cancelled;
    } else {
      auto stream = fTo.createOutputStream();
      if (builder.writeToStream(*stream, fProgress)) {
        fResult = CopyProgressComponent::Worker::Result::Success;
      } else {
        fResult = CopyProgressComponent::Worker::Result::Failed;
      }
    }
  }

private:
  AsyncUpdater *const fUpdater;
  File fFrom;
  File fTo;
  std::optional<CopyProgressComponent::Worker::Result> fResult;
  double *const fProgress;
};

CopyProgressComponent::CopyProgressComponent(ChooseOutputState const &state)
    : fState(state) {
  auto width = kWindowWidth;
  auto height = kWindowHeight;
  setSize(width, height);

  fLabel.reset(new Label("", TRANS("Saving...")));
  fLabel->setBounds(kMargin, kMargin, width - 2 * kMargin, kButtonBaseHeight);
  fLabel->setJustificationType(Justification::topLeft);
  addAndMakeVisible(*fLabel);

  fProgressBar.reset(new ProgressBar(fProgress));
  fProgressBar->setBounds(kMargin, kMargin + kButtonBaseHeight + kMargin,
                          width - 2 * kMargin, kButtonBaseHeight);
  addAndMakeVisible(*fProgressBar);

  if (state.fFormat == OutputFormat::Directory) {
    fCopyThread.reset(new CopyThread(this, state.fConvertState.fOutputDirectory,
                                     *state.fCopyDestination, &fProgress));
  } else {
    fCopyThread.reset(new ZipThread(this, state.fConvertState.fOutputDirectory,
                                    *state.fCopyDestination, &fProgress));
  }
  fCopyThread->startThread();
}

CopyProgressComponent::~CopyProgressComponent() {}

void CopyProgressComponent::paint(juce::Graphics &g) {}

void CopyProgressComponent::handleAsyncUpdate() {

  struct InvokeToChooseOutput : public ModalComponentManager::Callback {
    void modalStateFinished(int returnValue) override {
      JUCEApplication::getInstance()->invoke(gui::toChooseOutput, true);
    }
  };

  struct InvokeToChooseInput : public ModalComponentManager::Callback {
    void modalStateFinished(int returnValue) override {
      JUCEApplication::getInstance()->invoke(gui::toChooseInput, true);
    }
  };

  auto result = fCopyThread->result();
  if (!result || *result == CopyProgressComponent::Worker::Result::Failed) {
    NativeMessageBox::showMessageBoxAsync(
        AlertWindow::AlertIconType::WarningIcon, TRANS("Failed"),
        TRANS("Saving failed."), nullptr, new InvokeToChooseOutput);
  } else if (*result == CopyProgressComponent::Worker::Result::Cancelled) {
    NativeMessageBox::showMessageBoxAsync(
        AlertWindow::AlertIconType::InfoIcon, TRANS("Cancelled"),
        TRANS("Saving cancelled."), nullptr, new InvokeToChooseOutput);
    JUCEApplication::getInstance()->invoke(gui::toChooseOutput, true);
  } else {
    NativeMessageBox::showMessageBoxAsync(
        AlertWindow::AlertIconType::InfoIcon, TRANS("Completed"),
        TRANS("Saving completed."), nullptr, new InvokeToChooseInput);
    if (fState.fConvertState.fOutputDirectory.exists()) {
      fState.fConvertState.fOutputDirectory.deleteRecursively();
    }
  }
}
