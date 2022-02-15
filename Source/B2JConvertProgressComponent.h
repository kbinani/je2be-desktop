#pragma once

#include "CommandID.h"
#include "ComponentState.h"

namespace je2be::gui {

class TaskbarProgress;

class B2JConvertProgressComponent : public juce::Component,
                                    public B2JConvertStateProvider,
                                    public B2JConfigStateProvider,
                                    public B2JChooseInputStateProvider {
public:
  explicit B2JConvertProgressComponent(B2JConfigState const &configState);
  ~B2JConvertProgressComponent() override;

  void paint(juce::Graphics &) override;

  B2JConfigState getConfigState() const override {
    return fState.fConfigState;
  }

  B2JConvertState getConvertState() const override {
    return fState;
  }

  B2JChooseInputState getChooseInputState() const override {
    return fState.fConfigState.fInputState;
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
  std::unique_ptr<juce::TextButton> fCancelButton;
  B2JConvertState fState;
  std::unique_ptr<juce::Thread> fThread;
  std::shared_ptr<Updater> fUpdater;
  std::unique_ptr<juce::ProgressBar> fUnzipProgressBar;
  std::unique_ptr<juce::ProgressBar> fConversionProgressBar;
  double fUnzipProgress;
  double fConversionProgress;
  std::unique_ptr<juce::Label> fLabel;
  juce::CommandID fCommandWhenFinished = gui::toB2JChooseOutput;
  bool fFailed = false;
  std::unique_ptr<juce::TextEditor> fErrorMessage;
  std::unique_ptr<TaskbarProgress> fTaskbarProgress;
  bool fCancelRequested = false;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(B2JConvertProgressComponent)
};

} // namespace je2be::gui
