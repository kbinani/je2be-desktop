#pragma once

#include "ComponentState.h"
#include <optional>

namespace je2be::gui {

class TaskbarProgress;

class CopyProgressComponent : public juce::Component,
                              public juce::AsyncUpdater,
                              public J2BConvertStateProvider,
                              public juce::Timer {
public:
  explicit CopyProgressComponent(J2BChooseOutputState const &chooseOutputState);
  ~CopyProgressComponent() override;

  void paint(juce::Graphics &) override;

  void handleAsyncUpdate() override;

  J2BConvertState getConvertState() const override {
    return fState.fConvertState;
  }

  void timerCallback() override;

  class Worker : public juce::Thread {
  public:
    enum class Result {
      Success,
      Cancelled,
      Failed,
    };

    Worker(juce::String const &name) : Thread(name) {}
    virtual ~Worker() {}

    virtual std::optional<Result> result() const = 0;
  };

private:
  J2BChooseOutputState fState;
  std::unique_ptr<Worker> fCopyThread;
  std::unique_ptr<juce::Label> fLabel;
  std::unique_ptr<juce::ProgressBar> fProgressBar;
  double fProgress = -1;
  std::unique_ptr<TaskbarProgress> fTaskbarProgress;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CopyProgressComponent)
};

} // namespace je2be::gui
