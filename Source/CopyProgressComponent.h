#pragma once

#include "ComponentState.h"
#include <JuceHeader.h>

class CopyProgressComponent : public juce::Component, public AsyncUpdater {
public:
  explicit CopyProgressComponent(ChooseOutputState const &chooseOutputState);
  ~CopyProgressComponent() override;

  void paint(juce::Graphics &) override;

  void handleAsyncUpdate() override;

private:
  ChooseOutputState fState;
  std::unique_ptr<Thread> fCopyThread;
  std::unique_ptr<Label> fLabel;
  std::unique_ptr<ProgressBar> fProgressBar;
  double fProgress = -1;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CopyProgressComponent)
};
