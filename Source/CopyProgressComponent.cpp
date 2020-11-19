#include "CopyProgressComponent.h"
#include "CommandID.h"
#include "ComponentState.h"
#include "Constants.h"
#include <JuceHeader.h>

class CopyThread : public Thread {
public:
  CopyThread(AsyncUpdater *updater, File from, File to)
      : Thread("j2b::gui::CopyThread"), fUpdater(updater), fFrom(from),
        fTo(to) {}

  void run() override {
    fFrom.copyDirectoryTo(fTo);
    fUpdater->triggerAsyncUpdate();
  }

private:
  AsyncUpdater *const fUpdater;
  File fFrom;
  File fTo;
};

CopyProgressComponent::CopyProgressComponent(ChooseOutputState const &state)
    : fState(state) {
  auto width = kWindowWidth;
  auto height = kWindowHeight;
  setSize(width, height);

  fLabel.reset(new Label("", TRANS("Copying...")));
  fLabel->setBounds(kMargin, kMargin, width - 2 * kMargin, kButtonBaseHeight);
  fLabel->setJustificationType(Justification::topLeft);
  addAndMakeVisible(*fLabel);

  fProgressBar.reset(new ProgressBar(fProgress));
  fProgressBar->setBounds(kMargin, kMargin + kButtonBaseHeight + kMargin,
                          width - 2 * kMargin, kButtonBaseHeight);
  addAndMakeVisible(*fProgressBar);

  fCopyThread.reset(new CopyThread(this, state.fConvertState.fOutputDirectory,
                                   *state.fCopyDestinationDirectory));
  fCopyThread->startThread();
}

CopyProgressComponent::~CopyProgressComponent() {}

void CopyProgressComponent::paint(juce::Graphics &g) {}

void CopyProgressComponent::handleAsyncUpdate() {
  NativeMessageBox::showMessageBox(AlertWindow::AlertIconType::InfoIcon,
                                   TRANS("Completed"),
                                   TRANS("All files copied!"));
  if (fState.fConvertState.fOutputDirectory.exists()) {
    fState.fConvertState.fOutputDirectory.deleteRecursively();
  }
  JUCEApplication::getInstance()->invoke(gui::toChooseInput, true);
}