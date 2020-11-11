#pragma once

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
  void resized() override;

  ConfigState getConfigState() const override { return fState.fConfigState; }
  ConvertState getConvertState() const override { return fState; }
  ChooseInputState getChooseInputState() const override {
    return fState.fConfigState.fInputState;
  }

  void onCancelButtonClicked();

  void onProgressUpdate();

  class Updater : public AsyncUpdater {
  public:
    void handleAsyncUpdate() override {
      auto target = fTarget.load();
      if (!target)
        return;
      target->onProgressUpdate();
    }

    std::atomic<ConvertProgressComponent *> fTarget;
  };

private:
  std::unique_ptr<TextButton> fCancelButton;
  ConvertState fState;
  std::unique_ptr<Thread> fThread;

  std::shared_ptr<Updater> fUpdater;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConvertProgressComponent)
};
