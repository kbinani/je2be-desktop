/*
  ==============================================================================

    ChooseOutputComponent.h
    Created: 12 Nov 2020 1:22:32am
    Author:  kbinani

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
 */
class ChooseOutputComponent : public juce::Component {
public:
  ChooseOutputComponent();
  ~ChooseOutputComponent() override;

  void paint(juce::Graphics &) override;
  void resized() override;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChooseOutputComponent)
};
