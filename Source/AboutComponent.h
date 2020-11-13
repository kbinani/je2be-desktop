#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class AboutComponent : public Component {
public:
  AboutComponent();
  void paint(Graphics &g) override;

private:
  std::unique_ptr<Drawable> fLogo;
  Array<std::unique_ptr<Component>> fLines;
};
