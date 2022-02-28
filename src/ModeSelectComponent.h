#pragma once

#include "TextButtonComponent.h"

namespace je2be::gui {

class ModeSelectComponent : public juce::Component {
public:
  explicit ModeSelectComponent();
  ~ModeSelectComponent() override;

  void paint(juce::Graphics &) override;

  void onAboutButtonClicked();
  void onJ2BClicked();
  void onB2JClicked();

private:
  std::unique_ptr<juce::Label> fLabel;
  std::unique_ptr<TextButtonComponent> fToJ2B;
  std::unique_ptr<TextButtonComponent> fToB2J;
  std::unique_ptr<TextButtonComponent> fAboutButton;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModeSelectComponent)
};

} // namespace je2be::gui
