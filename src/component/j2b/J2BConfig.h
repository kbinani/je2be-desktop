#pragma once

#include "ComponentState.h"

namespace je2be::gui::component {
class TextButton;
}

namespace je2be::gui::component::j2b {

class J2BConfig : public juce::Component,
                  public ChooseInputStateProvider,
                  public J2BConfigStateProvider,
                  public juce::Timer {
public:
  explicit J2BConfig(ChooseInputState const &inputState);
  ~J2BConfig() override;

  void paint(juce::Graphics &) override;

  std::optional<ChooseInputState> getChooseInputState() const override {
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
  std::unique_ptr<TextButton> fBackButton;
  std::unique_ptr<TextButton> fStartButton;
  J2BConfigState fState;
  std::unique_ptr<juce::Label> fDirectory;
  bool fOk = false;
  std::unique_ptr<juce::Label> fMessage;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(J2BConfig)
};

} // namespace je2be::gui::component::j2b
