#pragma once

#include "AsyncUpdaterWith.h"
#include "GameDirectory.h"

namespace je2be::desktop {

class GameDirectoryScanWorkerJava {
public:
  std::vector<GameDirectory> fGameDirectories;

private:
  std::weak_ptr<AsyncUpdaterWith<std::vector<GameDirectory>>> fOwner;
  std::atomic<bool> fAbort;

public:
  explicit GameDirectoryScanWorkerJava(std::weak_ptr<AsyncUpdaterWith<std::vector<GameDirectory>>> owner);

  void run();
  void signalThreadShouldExit();
  bool threadShouldExit() const;

private:
  void unsafeRun();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GameDirectoryScanWorkerJava)
};

} // namespace je2be::desktop
