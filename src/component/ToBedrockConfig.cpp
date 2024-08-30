#include "component/ToBedrockConfig.h"
#include "CommandID.h"
#include "Constants.h"
#include "component/TextButton.h"

using namespace juce;

namespace je2be::desktop::component {

ToBedrockConfig::ToBedrockConfig(ChooseInputState const &chooseInputState, int forwardCommand, int backwardCommand) : fState(chooseInputState), fForwardCommand(forwardCommand), fBackwardCommand(backwardCommand) {
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

ToBedrockConfig::~ToBedrockConfig() {}

void ToBedrockConfig::timerCallback() {
  stopTimer();
  fStartButton->setEnabled(true);
}

void ToBedrockConfig::paint(juce::Graphics &g) {}

void ToBedrockConfig::onStartButtonClicked() {
  JUCEApplication::getInstance()->invoke(fForwardCommand, true);
}

void ToBedrockConfig::onBackButtonClicked() {
  JUCEApplication::getInstance()->invoke(fBackwardCommand, true);
}

} // namespace je2be::desktop::component
