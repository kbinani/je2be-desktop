#include "ConfigComponent.h"
#include "CommandID.h"
#include "Constants.h"
#include <JuceHeader.h>

ConfigComponent::ConfigComponent(ChooseInputState const &chooseInputState)
    : fState(chooseInputState) {
  auto width = kWindowWidth;
  auto height = kWindowHeight;
  setSize(width, height);

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
  g.drawText("ConfigComponent", getLocalBounds(), juce::Justification::centred,
             true); // draw some placeholder text
}

void ConfigComponent::resized() {}

void ConfigComponent::onStartButtonClicked() {
  JUCEApplication::getInstance()->perform({gui::toConvert});
}

void ConfigComponent::onBackButtonClicked() {
  JUCEApplication::getInstance()->perform({gui::toChooseInput});
}
