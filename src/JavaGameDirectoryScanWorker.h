#pragma once

#include "AsyncUpdaterWith.h"
#include "GameDirectory.h"

namespace je2be::desktop {

class JavaGameDirectoryScanWorker {
public:
  std::vector<GameDirectory> fGameDirectories;

private:
  std::weak_ptr<AsyncUpdaterWith<std::vector<GameDirectory>>> fOwner;
  std::atomic<bool> fAbort;

public:
  explicit JavaGameDirectoryScanWorker(std::weak_ptr<AsyncUpdaterWith<std::vector<GameDirectory>>> owner);

  void run();
  void signalThreadShouldExit();
  bool threadShouldExit() const;

private:
  void unsafeRun();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JavaGameDirectoryScanWorker)
};

} // namespace je2be::desktop
