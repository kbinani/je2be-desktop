#pragma once

#include "ComponentState.h"
#include <optional>

namespace j2b::gui {

class CopyProgressComponent : public juce::Component,
                              public juce::AsyncUpdater,
                              public ConvertStateProvider {
public:
  explicit CopyProgressComponent(ChooseOutputState const &chooseOutputState);
  ~CopyProgressComponent() override;

  void paint(juce::Graphics &) override;

  void handleAsyncUpdate() override;

  ConvertState getConvertState() const override {
    return fState.fConvertState;
  }

  class Worker : public juce::Thread {
  public:
    enum class Result {
      Success,
      Cancelled,
      Failed,
    };

    Worker(juce::String const &name) : Thread(name) {}
    virtual ~Worker() {}

    virtual std::optional<Result> result() const = 0;
  };

private:
  ChooseOutputState fState;
  std::unique_ptr<Worker> fCopyThread;
  std::unique_ptr<juce::Label> fLabel;
  std::unique_ptr<juce::ProgressBar> fProgressBar;
  double fProgress = -1;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CopyProgressComponent)
};

} // namespace j2b::gui
