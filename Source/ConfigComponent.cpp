#include "ConfigComponent.h"
#include "CommandID.h"
#include "Constants.h"
#include <JuceHeader.h>

ConfigComponent::ConfigComponent(ChooseInputState const &chooseInputState)
    : fState(chooseInputState) {
  auto width = kWindowWidth;
  auto height = kWindowHeight;
  setSize(width, height);

  String label = (*fState.fInputState.fInputDirectory).getFullPathName();
  fDirectory.reset(new Label("", TRANS("Selected world:") + " " + label));
  fDirectory->setBounds(kMargin, kMargin, width - kMargin * 2,
                        kButtonBaseHeight);
  addAndMakeVisible(*fDirectory);

  fStartButton.reset(new TextButton(TRANS("Start")));
  fStartButton->setBounds(width - kMargin - kButtonMinWidth,
                          height - kMargin - kButtonBaseHeight, kButtonMinWidth,
                          kButtonBaseHeight);
  fStartButton->onClick = [this]() { onStartButtonClicked(); };
  addAndMakeVisible(*fStartButton);

  fBackButton.reset(new TextButton(TRANS("Back")));
  fBackButton->setBounds(kMargin, height - kMargin - kButtonBaseHeight,
                         kButtonMinWidth, kButtonBaseHeight);
  fBackButton->onClick = [this]() { onBackButtonClicked(); };
  addAndMakeVisible(*fBackButton);
}

ConfigComponent::~ConfigComponent() {}

void ConfigComponent::paint(juce::Graphics &g) {
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void ConfigComponent::onStartButtonClicked() {
  JUCEApplication::getInstance()->perform({gui::toConvert});
}

void ConfigComponent::onBackButtonClicked() {
  JUCEApplication::getInstance()->perform({gui::toChooseInput});
}
