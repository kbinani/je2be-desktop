#pragma once

#include "ComponentState.h"
#include <JuceHeader.h>

class ConfigComponent : public juce::Component,
                        public ChooseInputStateProvider,
                        public ConfigStateProvider {
public:
  explicit ConfigComponent(ChooseInputState const &inputState);
  ~ConfigComponent() override;

  void paint(juce::Graphics &) override;

  ChooseInputState getChooseInputState() const override {
    return fState.fInputState;
  }
  ConfigState getConfigState() const override { return fState; }

private:
  void onBackButtonClicked();
  void onStartButtonClicked();

private:
  std::unique_ptr<TextButton> fBackButton;
  std::unique_ptr<TextButton> fStartButton;
  ConfigState fState;
  std::unique_ptr<Label> fDirectory;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConfigComponent)
};
