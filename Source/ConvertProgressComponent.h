/*
  ==============================================================================

    ConvertProgressComponent.h
    Created: 12 Nov 2020 1:22:20am
    Author:  kbinani

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
 */
class ConvertProgressComponent : public juce::Component {
public:
  ConvertProgressComponent();
  ~ConvertProgressComponent() override;

  void paint(juce::Graphics &) override;
  void resized() override;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConvertProgressComponent)
};
