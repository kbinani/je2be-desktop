#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace je2be::gui {

class DrawableTextComponent : public juce::Component {
public:
  explicit DrawableTextComponent(juce::String text, float fontSize) : fString(text) {
    auto label = std::make_unique<juce::Label>();
    auto font = label->getFont().withHeight(fontSize);
    fString.setFont(font);
    fString.setColour(label->getLookAndFeel().findColour(juce::Label::textColourId));
    fString.setJustification(juce::Justification::centredTop);
    fMinHeight = font.getHeight();
    setInterceptsMouseClicks(false, false);
  }

  void paint(juce::Graphics &g) override {
    fWidthWhenLayout = updateLayout();
    fLayout->draw(g, juce::Rectangle<float>(0, 0, getWidth(), getHeight()));
  }

  void resized() override {
    fWidthWhenLayout = updateLayout();
  }

  void shrinkToFit() {
    auto bounds = getBounds();
    setBounds(bounds.getX(), bounds.getY(), getWidth(), getTextHeight());
  }

private:
  int getTextHeight() {
    updateLayout();
    float f = fLayout->getHeight();
    return (std::max)((int)ceilf(f), fMinHeight);
  }

  int updateLayout() {
    int width = getWidth();
    if (width == fWidthWhenLayout && fLayout) {
      return width;
    }
    fLayout.reset(new juce::TextLayout());
    fLayout->createLayout(fString, width);
    return width;
  }

private:
  juce::AttributedString fString;
  std::unique_ptr<juce::TextLayout> fLayout;
  int fWidthWhenLayout = 0;
  int fMinHeight;
};

} // namespace je2be::gui
