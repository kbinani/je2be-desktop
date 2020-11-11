#pragma once

#include "ComponentState.h"
#include <JuceHeader.h>

class CopyProgressComponent : public juce::Component, public AsyncUpdater {
public:
  explicit CopyProgressComponent(ChooseOutputState const &chooseOutputState);
  ~CopyProgressComponent() override;

  void paint(juce::Graphics &) override;
  void resized() override;

  void handleAsyncUpdate() override;

private:
  std::unique_ptr<TextButton> fCancelButton;
  ChooseOutputState fState;
  std::unique_ptr<Thread> fCopyThread;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CopyProgressComponent)
};
