#include "CopyProgressComponent.h"
#include "CommandID.h"
#include "ComponentState.h"
#include "Constants.h"
#include <JuceHeader.h>

class CopyThread : public CopyProgressComponent::Worker {
public:
  CopyThread(AsyncUpdater *updater, File from, File to)
      : CopyProgressComponent::Worker("j2b::gui::CopyThread"),
        fUpdater(updater), fFrom(from), fTo(to) {}

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
    if (fFrom.copyDirectoryTo(fTo)) {
      fResult = CopyProgressComponent::Worker::Result::Success;
    } else {
      fResult = CopyProgressComponent::Worker::Result::Failed;
    }
  }

private:
  AsyncUpdater *const fUpdater;
  File fFrom;
  File fTo;
  std::optional<CopyProgressComponent::Worker::Result> fResult;
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
                                     *state.fCopyDestination));
  } else {
    fCopyThread.reset(new ZipThread(this, state.fConvertState.fOutputDirectory,
                                    *state.fCopyDestination, &fProgress));
  }
  fCopyThread->startThread();
}

CopyProgressComponent::~CopyProgressComponent() {}

void CopyProgressComponent::paint(juce::Graphics &g) {}

void CopyProgressComponent::handleAsyncUpdate() {

  auto result = fCopyThread->result();
  if (!result || *result == CopyProgressComponent::Worker::Result::Failed) {
    NativeMessageBox::showMessageBox(AlertWindow::AlertIconType::WarningIcon,
                                     TRANS("Failed"), TRANS("Saving failed."));
    JUCEApplication::getInstance()->invoke(gui::toChooseOutput, true);
  } else if (*result == CopyProgressComponent::Worker::Result::Cancelled) {
    NativeMessageBox::showMessageBox(AlertWindow::AlertIconType::InfoIcon,
                                     TRANS("Cancelled"),
                                     TRANS("Saving cancelled."));
    JUCEApplication::getInstance()->invoke(gui::toChooseOutput, true);
  } else {
    NativeMessageBox::showMessageBox(AlertWindow::AlertIconType::InfoIcon,
                                     TRANS("Completed"),
                                     TRANS("Saving completed."));
    if (fState.fConvertState.fOutputDirectory.exists()) {
      fState.fConvertState.fOutputDirectory.deleteRecursively();
    }
    JUCEApplication::getInstance()->invoke(gui::toChooseInput, true);
  }
}