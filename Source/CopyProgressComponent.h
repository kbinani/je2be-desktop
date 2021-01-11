#pragma once

#include "ComponentState.h"
#include <JuceHeader.h>

class CopyProgressComponent : public juce::Component,
                              public AsyncUpdater,
                              public ConvertStateProvider {
public:
  explicit CopyProgressComponent(ChooseOutputState const &chooseOutputState);
  ~CopyProgressComponent() override;

  void paint(juce::Graphics &) override;

  void handleAsyncUpdate() override;

  ConvertState getConvertState() const override { return fState.fConvertState; }

  class Worker : public Thread {
  public:
    enum class Result {
      Success,
      Cancelled,
      Failed,
    };

    Worker(String const &name) : Thread(name) {}
    virtual ~Worker() {}

    virtual std::optional<Result> result() const = 0;
  };

private:
  ChooseOutputState fState;
  std::unique_ptr<Worker> fCopyThread;
  std::unique_ptr<Label> fLabel;
  std::unique_ptr<ProgressBar> fProgressBar;
  double fProgress = -1;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CopyProgressComponent)
};
