#pragma once

#include "ComponentState.h"
#include "TextButtonComponent.h"

namespace je2be::gui::component::j2b {

class J2BConfigComponent : public juce::Component,
                           public J2BChooseInputStateProvider,
                           public J2BConfigStateProvider,
                           public juce::Timer {
public:
  explicit J2BConfigComponent(J2BChooseInputState const &inputState);
  ~J2BConfigComponent() override;

  void paint(juce::Graphics &) override;

  J2BChooseInputState getChooseInputState() const override {
    return fState.fInputState;
  }

  J2BConfigState getConfigState() const override {
    return fState;
  }

  void timerCallback() override;

private:
  void onBackButtonClicked();
  void onStartButtonClicked();

private:
  std::unique_ptr<TextButtonComponent> fBackButton;
  std::unique_ptr<TextButtonComponent> fStartButton;
  J2BConfigState fState;
  std::unique_ptr<juce::Label> fDirectory;
  bool fOk = false;
  std::unique_ptr<juce::Label> fMessage;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(J2BConfigComponent)
};

} // namespace je2be::gui::j2b
