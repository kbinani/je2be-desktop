#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class AboutComponent : public Component, public Timer {
public:
  AboutComponent();
  void paint(Graphics &g) override;

  void timerCallback() override;
  void mouseDown(MouseEvent const &) override;

private:
  std::unique_ptr<Drawable> fLogo;
  std::vector<String> fHeaderLines;
  std::vector<String> fLines;
  std::chrono::high_resolution_clock::duration fScrollDuration;
  bool fScrolling = true;
  std::chrono::high_resolution_clock::time_point fLastTick;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AboutComponent)
};
