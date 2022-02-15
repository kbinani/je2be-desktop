#pragma once

#include "ComponentState.h"
#include <optional>

namespace je2be::gui {
class TaskbarProgress;
}

namespace je2be::gui::j2b {

class J2BCopyProgressComponent : public juce::Component,
                                 public juce::AsyncUpdater,
                                 public J2BConvertStateProvider,
                                 public juce::Timer {
public:
  explicit J2BCopyProgressComponent(J2BChooseOutputState const &chooseOutputState);
  ~J2BCopyProgressComponent() override;

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

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(J2BCopyProgressComponent)
};

} // namespace je2be::gui::j2b
