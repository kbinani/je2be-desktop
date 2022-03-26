#pragma once

#include "CommandID.h"
#include "ComponentState.h"

namespace je2be::desktop {
class TaskbarProgress;
}

namespace je2be::desktop::component {
class TextButton;
}

namespace je2be::desktop::component::b2j {

class B2JConvertProgress : public juce::Component,
                           public JavaConvertedStateProvider,
                           public B2JConfigStateProvider,
                           public ChooseInputStateProvider {
public:
  explicit B2JConvertProgress(B2JConfigState const &configState);
  ~B2JConvertProgress() override;

  void paint(juce::Graphics &) override;

  B2JConfigState getConfigState() const override {
    return fConfigState;
  }

  std::optional<JavaConvertedState> getConvertedState() const override {
    return fState;
  }

  std::optional<ChooseInputState> getChooseInputState() const override {
    return fConfigState.fInputState;
  }

  void onCancelButtonClicked();

  enum class Phase {
    Unzip = 1,
    Conversion = 2,
    Done = 3,
    Error = -1,
  };

  void onProgressUpdate(Phase phase, double done, double total);

  class Updater;

private:
  std::unique_ptr<TextButton> fCancelButton;
  B2JConfigState fConfigState;
  juce::File fOutputDirectory;
  std::optional<JavaConvertedState> fState;
  std::unique_ptr<juce::Thread> fThread;
  std::shared_ptr<Updater> fUpdater;
  std::unique_ptr<juce::ProgressBar> fUnzipOrCopyProgressBar;
  std::unique_ptr<juce::ProgressBar> fConversionProgressBar;
  double fUnzipOrCopyProgress;
  double fConversionProgress;
  std::unique_ptr<juce::Label> fLabel;
  juce::CommandID fCommandWhenFinished = commands::toChooseJavaOutput;
  bool fFailed = false;
  std::unique_ptr<juce::TextEditor> fErrorMessage;
  std::unique_ptr<TaskbarProgress> fTaskbarProgress;
  bool fCancelRequested = false;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(B2JConvertProgress)
};

} // namespace je2be::desktop::component::b2j
