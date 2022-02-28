#pragma once

#include "component/DrawableText.h"

namespace je2be::gui::component {

class AboutComponent : public juce::Component, public juce::Timer {
public:
  AboutComponent();

  void mouseDown(juce::MouseEvent const &) override;

  void timerCallback() override;

private:
  int startScrollFrom(int startY, double startSpeed);

private:
  std::unique_ptr<juce::Component> fScrollContents;
  std::unique_ptr<juce::Component> fHeader;
  std::unique_ptr<juce::ImageComponent> fLogoComponent;
  std::unique_ptr<DrawableTextComponent> fAppName;
  std::unique_ptr<DrawableTextComponent> fAppVersion;
  std::vector<std::shared_ptr<DrawableTextComponent>> fParagraphs;
  std::unique_ptr<juce::ComponentAnimator> fAnimator;

  int fNextScrollStartY;
  double fNextStartSpeed;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AboutComponent)
};

} // namespace je2be::gui
