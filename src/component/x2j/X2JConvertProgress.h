#pragma once

#include "CommandID.h"
#include "ComponentState.h"

namespace je2be::gui {
class TaskbarProgress;
}

namespace je2be::gui::component {
class TextButton;
}

namespace je2be::gui::component::x2j {

class X2JConvertProgress : public juce::Component,
                           public JavaConvertedStateProvider,
                           public X2JConfigStateProvider,
                           public ChooseInputStateProvider {
public:
  explicit X2JConvertProgress(X2JConfigState const &configState);
  ~X2JConvertProgress() override;

  void paint(juce::Graphics &) override;

  X2JConfigState getConfigState() const override {
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
    Conversion = 1,
    Done = 2,
    Error = -1,
  };

  void onProgressUpdate(Phase phase, double done, double total);

  class Updater;

private:
  std::unique_ptr<TextButton> fCancelButton;
  X2JConfigState fConfigState;
  std::optional<JavaConvertedState> fState;
  juce::File fOutputDirectory;
  std::unique_ptr<juce::Thread> fThread;
  std::shared_ptr<Updater> fUpdater;
  std::unique_ptr<juce::ProgressBar> fConversionProgressBar;
  double fConversionProgress;
  std::unique_ptr<juce::Label> fLabel;
  juce::CommandID fCommandWhenFinished = gui::toChooseJavaOutput;
  bool fFailed = false;
  std::unique_ptr<juce::TextEditor> fErrorMessage;
  std::unique_ptr<TaskbarProgress> fTaskbarProgress;
  bool fCancelRequested = false;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(X2JConvertProgress)
};

} // namespace je2be::gui::component::x2j
