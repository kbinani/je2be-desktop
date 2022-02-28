#pragma once

#include "component/TextButton.h"

namespace je2be::gui::component {

class ModeSelect : public juce::Component {
public:
  explicit ModeSelect();
  ~ModeSelect() override;

  void paint(juce::Graphics &) override;

  void onAboutButtonClicked();
  void onJ2BClicked();
  void onB2JClicked();

private:
  std::unique_ptr<juce::Label> fLabel;
  std::unique_ptr<TextButton> fToJ2B;
  std::unique_ptr<TextButton> fToB2J;
  std::unique_ptr<TextButton> fAboutButton;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModeSelect)
};

} // namespace je2be::gui::component
