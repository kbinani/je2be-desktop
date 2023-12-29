#pragma once

#include "AsyncUpdaterWith.h"
#include "GameDirectory.h"

namespace je2be::desktop {

class Xbox360GameDirectoryScanWorker {
public:
  std::vector<GameDirectory> fGameDirectories;

private:
  std::weak_ptr<AsyncUpdaterWith<std::vector<GameDirectory>>> fOwner;
  std::atomic<bool> fAbort;

public:
  explicit Xbox360GameDirectoryScanWorker(std::weak_ptr<AsyncUpdaterWith<std::vector<GameDirectory>>> owner);

  void run();
  void signalThreadShouldExit();

private:
  void unsafeRun();
  void lookupRoot(juce::File root, std::vector<GameDirectory> &buffer);
  void lookupContent(juce::File content, std::vector<GameDirectory> &buffer);
  void lookupContentChild(juce::File child, std::vector<GameDirectory> &buffer);
  bool threadShouldExit() const;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Xbox360GameDirectoryScanWorker)
};

} // namespace je2be::desktop
