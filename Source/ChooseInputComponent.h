#pragma once

#include <JuceHeader.h>

class ChooseInputComponent : public juce::Component {
public:
  ChooseInputComponent();
  ~ChooseInputComponent() override;

  void paint(juce::Graphics &) override;
  void resized() override;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChooseInputComponent)
};
