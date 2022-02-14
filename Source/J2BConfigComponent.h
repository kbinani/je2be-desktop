#pragma once

#include "ComponentState.h"

namespace je2be::gui {

class ConfigComponent : public juce::Component,
                        public J2BChooseInputStateProvider,
                        public J2BConfigStateProvider,
                        public juce::Timer {
public:
  explicit ConfigComponent(J2BChooseInputState const &inputState);
  ~ConfigComponent() override;

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
  std::unique_ptr<juce::TextButton> fBackButton;
  std::unique_ptr<juce::TextButton> fStartButton;
  J2BConfigState fState;
  std::unique_ptr<juce::Label> fDirectory;
  bool fOk = false;
  std::unique_ptr<juce::Label> fMessage;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConfigComponent)
};

} // namespace je2be::gui
