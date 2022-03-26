#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace je2be::desktop {

class TaskbarProgress {
public:
  TaskbarProgress();
  ~TaskbarProgress();

  enum class State {
    NoProgress,
    Indeterminate,
    Normal,
    Error,
  };

  void setState(State state);

  void update(double progress);

private:
  class Impl;
  std::unique_ptr<Impl> fImpl;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TaskbarProgress)
};

} // namespace je2be::desktop
