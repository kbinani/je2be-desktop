#pragma once

#include "ComponentState.h"

namespace je2be::gui {
class TaskbarProgress;
}

namespace je2be::gui::component {

class CopyJavaArtifactProgress : public juce::Component,
                                 public juce::AsyncUpdater,
                                 public JavaConvertedStateProvider,
                                 public juce::Timer {
public:
  explicit CopyJavaArtifactProgress(JavaOutputChoosenState const &chooseOutputState);
  ~CopyJavaArtifactProgress() override;

  void paint(juce::Graphics &) override;

  void handleAsyncUpdate() override;

  std::optional<JavaConvertedState> getConvertedState() const override {
    return fState.fConvertedState;
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
  JavaOutputChoosenState fState;
  std::unique_ptr<Worker> fCopyThread;
  std::unique_ptr<juce::Label> fLabel;
  std::unique_ptr<juce::ProgressBar> fProgressBar;
  double fProgress = -1;
  std::unique_ptr<TaskbarProgress> fTaskbarProgress;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CopyJavaArtifactProgress)
};

} // namespace je2be::gui::component
