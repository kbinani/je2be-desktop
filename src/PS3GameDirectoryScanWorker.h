#pragma once

#include "AsyncUpdaterWith.h"
#include "GameDirectory.h"

namespace je2be::desktop {

class PS3GameDirectoryScanWorker {
public:
  std::vector<GameDirectory> fGameDirectories;

private:
  std::weak_ptr<AsyncUpdaterWith<std::vector<GameDirectory>>> fOwner;
  std::atomic<bool> fAbort;

public:
  explicit PS3GameDirectoryScanWorker(std::weak_ptr<AsyncUpdaterWith<std::vector<GameDirectory>>> owner);

  void run();
  void signalThreadShouldExit();

private:
  void unsafeRun();
  void lookupRoot(juce::File root, std::vector<GameDirectory> &buffer);
  void lookupPS3(juce::File content, std::vector<GameDirectory> &buffer);
  void lookupSAVEDATA(juce::File child, std::vector<GameDirectory> &buffer);
  bool threadShouldExit() const;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PS3GameDirectoryScanWorker)
};

} // namespace je2be::desktop
