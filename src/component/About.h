#pragma once

#include "component/DrawableText.h"

namespace je2be::gui::component {

class About : public juce::Component, public juce::Timer {
public:
  About();

  void mouseDown(juce::MouseEvent const &) override;

  void timerCallback() override;

private:
  int startScrollFrom(int startY, double startSpeed);

private:
  std::unique_ptr<juce::Component> fScrollContents;
  std::unique_ptr<juce::Component> fHeader;
  std::unique_ptr<juce::ImageComponent> fLogoComponent;
  std::unique_ptr<DrawableText> fAppName;
  std::unique_ptr<DrawableText> fAppVersion;
  std::vector<std::shared_ptr<DrawableText>> fParagraphs;
  std::unique_ptr<juce::ComponentAnimator> fAnimator;

  int fNextScrollStartY;
  double fNextStartSpeed;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(About)
};

} // namespace je2be::gui::component
