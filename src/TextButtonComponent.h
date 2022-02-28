#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace je2be::gui {

class TextButtonComponent : public juce::TextButton {
  using super = juce::TextButton;

public:
  explicit TextButtonComponent(juce::String text) : juce::TextButton(text) {
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
};

} // namespace je2be::gui
