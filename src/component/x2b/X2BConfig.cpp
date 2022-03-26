#include "component/x2b/X2BConfig.h"
#include "CommandID.h"
#include "Constants.h"
#include "component/TextButton.h"

using namespace juce;

namespace je2be::gui::component::x2b {

X2BConfig::X2BConfig(ChooseInputState const &chooseInputState) : fState(chooseInputState) {
  auto width = kWindowWidth;
  auto height = kWindowHeight;
  setSize(width, height);

  String label = fState.fInputState.fInput.getFullPathName();
  fDirectory.reset(new Label("", TRANS("Selected world:") + " " + label));
  fDirectory->setBounds(kMargin, kMargin, width - kMargin * 2, kButtonBaseHeight);
  fDirectory->setJustificationType(Justification::topLeft);
  addAndMakeVisible(*fDirectory);

  fStartButton.reset(new component::TextButton(TRANS("Start")));
  fStartButton->setBounds(width - kMargin - kButtonMinWidth, height - kMargin - kButtonBaseHeight, kButtonMinWidth, kButtonBaseHeight);
  fStartButton->setEnabled(false);
  fStartButton->onClick = [this]() { onStartButtonClicked(); };
  addAndMakeVisible(*fStartButton);

  fBackButton.reset(new component::TextButton(TRANS("Back")));
  fBackButton->setBounds(kMargin, height - kMargin - kButtonBaseHeight, kButtonMinWidth, kButtonBaseHeight);
  fBackButton->onClick = [this]() { onBackButtonClicked(); };
  addAndMakeVisible(*fBackButton);

  startTimer(1000);
}

X2BConfig::~X2BConfig() {}

void X2BConfig::timerCallback() {
  stopTimer();
  fStartButton->setEnabled(true);
}

void X2BConfig::paint(juce::Graphics &g) {}

void X2BConfig::onStartButtonClicked() {
  JUCEApplication::getInstance()->invoke(gui::toXbox360ToBedrockConvert, true);
}

void X2BConfig::onBackButtonClicked() {
  JUCEApplication::getInstance()->invoke(gui::toChooseXbox360InputToBedrock, true);
}

} // namespace je2be::gui::component::x2b
