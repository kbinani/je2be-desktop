#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

class AboutComponent : public juce::Component, public juce::Timer {
public:
  AboutComponent();
  void paint(juce::Graphics &g) override;

  void timerCallback() override;
  void mouseDown(juce::MouseEvent const &) override;

private:
  std::unique_ptr<juce::Drawable> fLogo;
  std::vector<juce::String> fHeaderLines;
  std::vector<juce::String> fLines;
  std::chrono::high_resolution_clock::duration fScrollDuration;
  bool fScrolling = true;
  std::chrono::high_resolution_clock::time_point fLastTick;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AboutComponent)
};
