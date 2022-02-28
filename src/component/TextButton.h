#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace je2be::gui::component {

class TextButton : public juce::TextButton {
  using super = juce::TextButton;

public:
  explicit TextButton(juce::String text) : juce::TextButton(text) {
    updateMouseCursor();
  }

  void enablementChanged() override {
    super::enablementChanged();
    updateMouseCursor();
  }

private:
  void updateMouseCursor() {
    if (isEnabled()) {
      setMouseCursor(juce::MouseCursor::PointingHandCursor);
    } else {
      setMouseCursor(juce::MouseCursor::NormalCursor);
    }
  }

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TextButton)
};

} // namespace je2be::gui::component
