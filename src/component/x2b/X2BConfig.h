#pragma once

#include "ComponentState.h"

namespace je2be::gui::component {
class TextButton;
}

namespace je2be::gui::component::x2b {

class X2BConfig : public juce::Component,
                  public ChooseInputStateProvider,
                  public X2BConfigStateProvider,
                  public juce::Timer {
public:
  explicit X2BConfig(ChooseInputState const &inputState);
  ~X2BConfig() override;

  void paint(juce::Graphics &) override;

  std::optional<ChooseInputState> getChooseInputState() const override {
    return fState.fInputState;
  }

  X2BConfigState getConfigState() const override {
    return fState;
  }

  void timerCallback() override;

private:
  void onBackButtonClicked();
  void onStartButtonClicked();

private:
  std::unique_ptr<TextButton> fBackButton;
  std::unique_ptr<TextButton> fStartButton;
  X2BConfigState fState;
  std::unique_ptr<juce::Label> fDirectory;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(X2BConfig)
};

} // namespace je2be::gui::component::x2b
