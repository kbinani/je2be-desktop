#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class AboutComponent : public Component, public Timer {
public:
  AboutComponent();
  void paint(Graphics &g) override;

  void timerCallback() override;

private:
  std::unique_ptr<Drawable> fLogo;
  std::vector<String> fHeaderLines;
  std::vector<String> fLines;
  std::chrono::time_point<std::chrono::high_resolution_clock> fStartTime;
};
