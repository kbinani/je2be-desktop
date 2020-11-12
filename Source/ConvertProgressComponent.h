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

  class Updater : public AsyncUpdater {
    struct Entry {
      int fPhase;
      double fDone;
      double fTotal;
    };

  public:
    void trigger(int phase, double done, double total) {
      Entry entry;
      entry.fPhase = phase;
      entry.fDone = done;
      entry.fTotal = total;
      {
        std::lock_guard<std::mutex> lk(fMut);
        fQueue.push_back(entry);
      }
      triggerAsyncUpdate();
    }

    void handleAsyncUpdate() override {
      std::deque<Entry> copy;
      {
        std::lock_guard<std::mutex> lk(fMut);
        copy.swap(fQueue);
      }
      auto target = fTarget.load();
      if (!target)
        return;
      for (auto const &e : copy) {
        target->onProgressUpdate(e.fPhase, e.fDone, e.fTotal);
      }
    }

    std::atomic<ConvertProgressComponent *> fTarget;

  private:
    std::deque<Entry> fQueue;
    std::mutex fMut;
  };

private:
  std::unique_ptr<TextButton> fCancelButton;
  ConvertState fState;
  std::unique_ptr<Thread> fThread;
  std::shared_ptr<Updater> fUpdater;
  std::unique_ptr<ProgressBar> fProgressBar;
  double fProgress = 0;
  std::unique_ptr<Label> fLabel;
  CommandID fCommandWhenFinished = gui::toChooseOutput;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConvertProgressComponent)
};
