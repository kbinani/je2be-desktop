#pragma once

#include "CommandID.h"
#include "ComponentState.h"

namespace je2be::gui {

class TaskbarProgress;

class J2BConvertProgressComponent : public juce::Component,
                                    public J2BConvertStateProvider,
                                    public J2BConfigStateProvider,
                                    public J2BChooseInputStateProvider {
public:
  explicit J2BConvertProgressComponent(J2BConfigState const &configState);
  ~J2BConvertProgressComponent() override;

  void paint(juce::Graphics &) override;

  J2BConfigState getConfigState() const override {
    return fState.fConfigState;
  }

  J2BConvertState getConvertState() const override {
    return fState;
  }

  J2BChooseInputState getChooseInputState() const override {
    return fState.fConfigState.fInputState;
  }

  void onCancelButtonClicked();

  void onProgressUpdate(int phase, double done, double total);

  class Updater;

private:
  std::unique_ptr<juce::TextButton> fCancelButton;
  J2BConvertState fState;
  std::unique_ptr<juce::Thread> fThread;
  std::shared_ptr<Updater> fUpdater;
  std::unique_ptr<juce::ProgressBar> fConversionProgressBar;
  std::unique_ptr<juce::ProgressBar> fCompactionProgressBar;
  double fConversionProgress = 0;
  double fCompactionProgress = 0;
  std::unique_ptr<juce::Label> fLabel;
  juce::CommandID fCommandWhenFinished = gui::toChooseOutput;
  bool fFailed = false;
  std::unique_ptr<juce::TextEditor> fErrorMessage;
  std::unique_ptr<TaskbarProgress> fTaskbarProgress;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(J2BConvertProgressComponent)
};

} // namespace je2be::gui
