#include "B2JConfigComponent.h"
#include "CommandID.h"
#include "Constants.h"

using namespace juce;

namespace je2be::gui::b2j {

B2JConfigComponent::B2JConfigComponent(B2JChooseInputState const &chooseInputState) : fState(chooseInputState) {
  auto width = kWindowWidth;
  auto height = kWindowHeight;
  setSize(width, height);

  String label = (*fState.fInputState.fInputFileOrDirectory).getFullPathName();
  fFileOrDirectory.reset(new Label("", TRANS("Selected world:") + " " + label));
  fFileOrDirectory->setBounds(kMargin, kMargin, width - kMargin * 2, kButtonBaseHeight);
  fFileOrDirectory->setJustificationType(Justification::topLeft);
  addAndMakeVisible(*fFileOrDirectory);

  fStartButton.reset(new TextButton(TRANS("Start")));
  fStartButton->setBounds(width - kMargin - kButtonMinWidth, height - kMargin - kButtonBaseHeight, kButtonMinWidth, kButtonBaseHeight);
  fStartButton->setEnabled(false);
  fStartButton->onClick = [this]() { onStartButtonClicked(); };
  addAndMakeVisible(*fStartButton);

  fBackButton.reset(new TextButton(TRANS("Back")));
  fBackButton->setBounds(kMargin, height - kMargin - kButtonBaseHeight, kButtonMinWidth, kButtonBaseHeight);
  fBackButton->setMouseCursor(MouseCursor::PointingHandCursor);
  fBackButton->onClick = [this]() { onBackButtonClicked(); };
  addAndMakeVisible(*fBackButton);

  //TODO: check given file or directory
  fOk = true;

  if (fOk) {
    fMessage.reset(new Label("", ""));
  } else {
    fMessage.reset(new Label("", TRANS("There doesn't seem to be any Minecraft save data "
                                       "in the specified directory.")));
    fMessage->setColour(Label::textColourId, kErrorTextColor);
  }
  fMessage->setBounds(kMargin, kMargin + kButtonBaseHeight + kMargin, width - 2 * kMargin, kButtonBaseHeight);
  addAndMakeVisible(*fMessage);

  startTimer(1000);
}

B2JConfigComponent::~B2JConfigComponent() {}

void B2JConfigComponent::timerCallback() {
  stopTimer();
  fStartButton->setEnabled(fOk);
  if (fOk) {
    fStartButton->setMouseCursor(MouseCursor::PointingHandCursor);
  }
}

void B2JConfigComponent::paint(juce::Graphics &g) {}

void B2JConfigComponent::onStartButtonClicked() {
  JUCEApplication::getInstance()->invoke(gui::toB2JConvert, true);
}

void B2JConfigComponent::onBackButtonClicked() {
  JUCEApplication::getInstance()->invoke(gui::toB2JChooseInput, true);
}

} // namespace je2be::gui::b2j
