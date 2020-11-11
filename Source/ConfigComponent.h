/*
  ==============================================================================

    ConfigComponent.h
    Created: 12 Nov 2020 1:22:09am
    Author:  kbinani

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
 */
class ConfigComponent : public juce::Component {
public:
  ConfigComponent();
  ~ConfigComponent() override;

  void paint(juce::Graphics &) override;
  void resized() override;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConfigComponent)
};
