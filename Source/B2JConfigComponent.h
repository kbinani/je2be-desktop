#pragma once

#include "ComponentState.h"

namespace je2be::gui {

class B2JConfigComponent : public juce::Component,
                           public B2JChooseInputStateProvider,
                           public B2JConfigStateProvider,
                           public juce::Timer {
public:
  explicit B2JConfigComponent(B2JChooseInputState const &inputState);
  ~B2JConfigComponent() override;

  void paint(juce::Graphics &) override;

  B2JChooseInputState getChooseInputState() const override {
    return fState.fInputState;
  }

  B2JConfigState getConfigState() const override {
    return fState;
  }

  void timerCallback() override;

private:
  void onBackButtonClicked();
  void onStartButtonClicked();

private:
  std::unique_ptr<juce::TextButton> fBackButton;
  std::unique_ptr<juce::TextButton> fStartButton;
  B2JConfigState fState;
  std::unique_ptr<juce::Label> fFileOrDirectory;
  bool fOk = false;
  std::unique_ptr<juce::Label> fMessage;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(B2JConfigComponent)
};

} // namespace je2be::gui
