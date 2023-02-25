#pragma once

#include "ComponentState.h"
#include "Status.hpp"

namespace je2be::desktop {
class TaskbarProgress;
}

namespace je2be::desktop::component {

class CopyBedrockArtifactProgress : public juce::Component,
                                    public juce::AsyncUpdater,
                                    public BedrockConvertedStateProvider,
                                    public juce::Timer {
public:
  explicit CopyBedrockArtifactProgress(BedrockOutputChoosenState const &chooseOutputState);
  ~CopyBedrockArtifactProgress() override;

  void paint(juce::Graphics &) override;

  void handleAsyncUpdate() override;

  std::optional<BedrockConvertedState> getConvertedState() const override {
    return fState.fConvertedState;
  }

  void timerCallback() override;

  class Worker : public juce::Thread {
  public:
    struct Result {
      enum class Type {
        Success,
        Cancelled,
        TooLargeOutput,
        Failed,
      };
      Type fType;
      Status fStatus;

      static Result Success() {
        return Result(Type::Success, Status::Ok());
      }

      static Result Cancelled() {
        return Result(Type::Cancelled, Status::Ok());
      }

      static Result Failed(Status status) {
        return Result(Type::Failed, status);
      }

      static Result TooLargeOutput() {
        return Result(Type::TooLargeOutput, Status::Ok());
      }

    private:
      Result(Type type, Status st) : fType(type), fStatus(st) {}
    };

    Worker(juce::String const &name) : Thread(name) {}
    virtual ~Worker() {}

    virtual std::optional<Result> result() const = 0;
  };

private:
  BedrockOutputChoosenState fState;
  std::unique_ptr<Worker> fCopyThread;
  std::unique_ptr<juce::Label> fLabel;
  std::unique_ptr<juce::ProgressBar> fProgressBar;
  double fProgress = -1;
  std::unique_ptr<TaskbarProgress> fTaskbarProgress;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CopyBedrockArtifactProgress)
};

} // namespace je2be::desktop::component
