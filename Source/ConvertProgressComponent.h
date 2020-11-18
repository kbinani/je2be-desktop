#pragma once

#include "CommandID.h"
#include "ComponentState.h"
#include <JuceHeader.h>

class ConvertProgressComponent : public juce::Component,
                                 public ConvertStateProvider,
                                 public ConfigStateProvider,
                                 public ChooseInputStateProvider {
public:
  explicit ConvertProgressComponent(ConfigState const &configState);
  ~ConvertProgressComponent() override;

  void paint(juce::Graphics &) override;

  ConfigState getConfigState() const override { return fState.fConfigState; }
  ConvertState getConvertState() const override { return fState; }
  ChooseInputState getChooseInputState() const override {
    return fState.fConfigState.fInputState;
  }

  void onCancelButtonClicked();

  void onProgressUpdate(int phase, double done, double total);

  class Updater;

private:
  std::unique_ptr<TextButton> fCancelButton;
  ConvertState fState;
  std::unique_ptr<Thread> fThread;
  std::shared_ptr<Updater> fUpdater;
  std::unique_ptr<ProgressBar> fProgressBar;
  double fProgress = 0;
  std::unique_ptr<Label> fLabel;
  CommandID fCommandWhenFinished = gui::toChooseOutput;
  bool fFailed = false;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConvertProgressComponent)
};
