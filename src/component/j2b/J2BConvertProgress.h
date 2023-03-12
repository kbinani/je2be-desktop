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

namespace je2be::desktop::component::j2b {

class J2BConvertProgress : public juce::Component,
                           public BedrockConvertedStateProvider,
                           public J2BConfigStateProvider,
                           public ChooseInputStateProvider {
public:
  enum class Phase : int {
    Error = -1,
    Convert = 0,
    PostProcess = 1,
    LevelDBCompaction = 2,
    Done = 3,
  };

  struct UpdateQueue {
    Phase fPhase;
    double fProgress;
    uint64_t fNumConvertedChunks;
    Status fStatus;
  };

  explicit J2BConvertProgress(J2BConfigState const &configState);
  ~J2BConvertProgress() override;

  void paint(juce::Graphics &) override;

  J2BConfigState getConfigState() const override {
    return fConfigState;
  }

  std::optional<BedrockConvertedState> getConvertedState() const override {
    return fState;
  }

  std::optional<ChooseInputState> getChooseInputState() const override {
    return fConfigState.fInputState;
  }

  void onCancelButtonClicked();

  void onProgressUpdate(Phase phase, double progress, uint64_t numConvertedChunks, Status status);

private:
  std::unique_ptr<TextButton> fCancelButton;
  J2BConfigState fConfigState;
  std::optional<BedrockConvertedState> fState;
  juce::File fOutputDirectory;
  std::unique_ptr<juce::Thread> fThread;
  std::shared_ptr<AsyncHandler<UpdateQueue>> fUpdater;
  std::unique_ptr<juce::ProgressBar> fConversionProgressBar;
  std::unique_ptr<juce::ProgressBar> fPostProcessProgressBar;
  std::unique_ptr<juce::ProgressBar> fCompactionProgressBar;
  double fConversionProgress = 0;
  double fPostProcessProgress = 0;
  double fCompactionProgress = 0;
  std::unique_ptr<juce::Label> fLabel;
  juce::CommandID fCommandWhenFinished = commands::toChooseBedrockOutput;
  bool fFailed = false;
  std::unique_ptr<juce::TextEditor> fErrorMessage;
  std::unique_ptr<TaskbarProgress> fTaskbarProgress;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(J2BConvertProgress)
};

} // namespace je2be::desktop::component::j2b
