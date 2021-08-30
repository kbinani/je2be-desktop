#pragma once

#include "ComponentState.h"

class ConfigComponent : public juce::Component,
                        public ChooseInputStateProvider,
                        public ConfigStateProvider,
                        public juce::Timer {
public:
  explicit ConfigComponent(ChooseInputState const &inputState);
  ~ConfigComponent() override;

  void paint(juce::Graphics &) override;

  ChooseInputState getChooseInputState() const override {
    return fState.fInputState;
  }

  ConfigState getConfigState() const override {
    return fState;
  }

  void timerCallback() override;

private:
  void onBackButtonClicked();
  void onStartButtonClicked();

private:
  std::unique_ptr<juce::TextButton> fBackButton;
  std::unique_ptr<juce::TextButton> fStartButton;
  ConfigState fState;
  std::unique_ptr<juce::Label> fDirectory;
  bool fOk = false;
  std::unique_ptr<juce::Label> fMessage;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConfigComponent)
};
