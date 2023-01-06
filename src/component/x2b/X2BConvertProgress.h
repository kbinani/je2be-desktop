#pragma once

#include "AsyncHandler.h"
#include "CommandID.h"
#include "ComponentState.h"
#include "Status.hpp"

namespace je2be::desktop {
class TaskbarProgress;
}

namespace je2be::desktop::component {
class TextButton;
}

namespace je2be::desktop::component::x2b {

class X2BConvertProgress : public juce::Component,
                           public BedrockConvertedStateProvider,
                           public X2BConfigStateProvider,
                           public ChooseInputStateProvider {
public:
  explicit X2BConvertProgress(X2BConfigState const &configState);
  ~X2BConvertProgress() override;

  void paint(juce::Graphics &) override;

  X2BConfigState getConfigState() const override {
    return fConfigState;
  }

  std::optional<BedrockConvertedState> getConvertedState() const override {
    return fState;
  }

  std::optional<ChooseInputState> getChooseInputState() const override {
    return fConfigState.fInputState;
  }

  void onCancelButtonClicked();

  enum class Phase {
    XboxToJavaConversion,
    JavaToBedrockConversion,
    JavaToBedrockCompaction,
    Done,
    Error,
  };

  struct UpdateQueue {
    Phase fPhase;
    double fProgress;
    uint64_t fNumConvertedChunks;
    Status fStatus;
  };

  void onProgressUpdate(Phase phase, double progress, uint64_t numConvertedChunks, Status status);

private:
  std::unique_ptr<TextButton> fCancelButton;
  X2BConfigState fConfigState;
  std::optional<BedrockConvertedState> fState;
  juce::File fOutputDirectory;
  std::unique_ptr<juce::Thread> fThread;
  std::shared_ptr<AsyncHandler<UpdateQueue>> fUpdater;
  std::unique_ptr<juce::ProgressBar> fXbox360ToJavaConversionProgressBar;
  std::unique_ptr<juce::ProgressBar> fJavaToBedrockConversionProgressBar;
  std::unique_ptr<juce::ProgressBar> fJavaToBedrockCompactionProgressBar;
  double fXbox360ToJavaConversionProgress = 0;
  double fJavaToBedrockConversionProgress = 0;
  double fJavaToBedrockCompactionProgress = 0;
  std::unique_ptr<juce::Label> fLabel;
  juce::CommandID fCommandWhenFinished = commands::toChooseBedrockOutput;
  bool fFailed = false;
  std::unique_ptr<juce::TextEditor> fErrorMessage;
  std::unique_ptr<TaskbarProgress> fTaskbarProgress;
  bool fCancelRequested = false;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(X2BConvertProgress)
};

} // namespace je2be::desktop::component::x2b
