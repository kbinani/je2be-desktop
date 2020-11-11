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
  {
    fCancelButton.reset(new TextButton(TRANS("Cancel")));
    fCancelButton->setBounds(kMargin, height - kMargin - kButtonBaseHeight,
                             kButtonMinWidth, kButtonBaseHeight);
    addAndMakeVisible(*fCancelButton);
  }

  fCopyThread.reset(new CopyThread(this, state.fConvertState.fOutputDirectory,
                                   *state.fCopyDestinationDirectory));
  fCopyThread->startThread();
}

CopyProgressComponent::~CopyProgressComponent() {}

void CopyProgressComponent::paint(juce::Graphics &g) {
  /* This demo code just fills the component's background and
     draws some placeholder text to get you started.

     You should replace everything in this method with your own
     drawing code..
  */

  g.fillAll(getLookAndFeel().findColour(
      juce::ResizableWindow::backgroundColourId)); // clear the background

  g.setColour(juce::Colours::grey);
  g.drawRect(getLocalBounds(), 1); // draw an outline around the component

  g.setColour(juce::Colours::white);
  g.setFont(14.0f);
  g.drawText("CopyProgressComponent", getLocalBounds(),
             juce::Justification::centred, true); // draw some placeholder text
}

void CopyProgressComponent::resized() {
  // This method is where you should set the bounds of any child
  // components that your component contains..
}

void CopyProgressComponent::handleAsyncUpdate() {
  NativeMessageBox::showMessageBox(AlertWindow::AlertIconType::InfoIcon,
                                   TRANS("Completed"),
                                   TRANS("All files copied!"));
  JUCEApplication::getInstance()->perform({gui::toChooseInput});
}