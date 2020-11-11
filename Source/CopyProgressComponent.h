/*
  ==============================================================================

    CopyProgressComponent.h
    Created: 12 Nov 2020 1:22:48am
    Author:  kbinani

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
 */
class CopyProgressComponent : public juce::Component {
public:
  CopyProgressComponent();
  ~CopyProgressComponent() override;

  void paint(juce::Graphics &) override;
  void resized() override;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CopyProgressComponent)
};
