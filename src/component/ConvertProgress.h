#pragma once

#include "Status.hpp"
#include "TaskbarProgress.h"
#include "component/TextButton.h"

#include <memory>

namespace je2be::desktop::component {

class ConvertProgress : public juce::Component, public juce::AsyncUpdater {
public:
  struct Progress {
    double fProgress;
    uint64_t fCount;
  };

protected:
  struct ProgressQueue {
    int fStep;
    Progress fProgress;
  };

  struct FailureQueue {
    Status fStatus;
  };

  struct FinishQueue {};

  using Queue = std::variant<ProgressQueue, FailureQueue, FinishQueue>;

  struct Characteristics {
    enum Unit {
      Chunk,
      Percent,
    };
    Unit fUnit;
    juce::String fLabel;
    juce::String fProgressBarLabel;
    juce::String fProgressBarExtraLabelFormat;
    Characteristics(Unit unit, juce::String const &label, juce::String progressBarLabel, juce::String progressBarExtraLabelFormat = "")
        : fUnit(unit), fLabel(label), fProgressBarLabel(progressBarLabel), fProgressBarExtraLabelFormat(progressBarExtraLabelFormat) {}
  };

  virtual int getProgressSteps() const = 0;
  virtual Characteristics getProgressCharacteristics(int step) const = 0;
  virtual void onCancelButtonClicked() = 0;
  virtual void startThread() = 0;
  virtual void onFinish() = 0;

  void handleAsyncUpdate() override;

public:
  ConvertProgress();
  virtual ~ConvertProgress() {}
  void parentHierarchyChanged() override;

  void notifyProgress(int step, Progress progress);
  void notifyError(Status status);
  void notifyFinished();

protected:
  bool fPrepared = false;
  std::unique_ptr<juce::Label> fLabel;
  std::unique_ptr<TextButton> fCancelButton;
  std::optional<Status> fFailure;
  bool fFinished = false;
  std::unique_ptr<juce::TextEditor> fErrorMessage;
  std::vector<std::unique_ptr<juce::ProgressBar>> fProgressBars;
  std::vector<double> fProgresses;
  std::unique_ptr<TaskbarProgress> fTaskbarProgress;
  int fStep = 0;
  std::deque<Queue> fQueue;
  std::mutex fMut;
  std::vector<Progress> fLast;
  std::unique_ptr<juce::Thread> fThread;
};

} // namespace je2be::desktop::component
