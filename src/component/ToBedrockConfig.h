#pragma once

#include "ComponentState.h"

namespace je2be::desktop::component {

class TextButton;

class ToBedrockConfig : public juce::Component,
                        public ChooseInputStateProvider,
                        public ToBedrockConfigStateProvider,
                        public juce::Timer {
public:
  ToBedrockConfig(ChooseInputState const &inputState, int forwardCommand, int backwardCommand);
  ~ToBedrockConfig() override;

  void paint(juce::Graphics &) override;

  std::optional<ChooseInputState> getChooseInputState() const override {
    return fState.fInputState;
  }

  ToBedrockConfigState getConfigState() const override {
    return fState;
  }

  void timerCallback() override;

private:
  void onBackButtonClicked();
  void onStartButtonClicked();

private:
  std::unique_ptr<TextButton> fBackButton;
  std::unique_ptr<TextButton> fStartButton;
  ToBedrockConfigState fState;
  std::unique_ptr<juce::Label> fDirectory;
  int const fForwardCommand;
  int const fBackwardCommand;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ToBedrockConfig)
};

} // namespace je2be::desktop::component
