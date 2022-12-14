#pragma once

#include "GameDirectory.h"

namespace je2be::desktop {

class GameDirectoryScanThreadXbox360 : public juce::Thread {
public:
  std::vector<GameDirectory> fGameDirectories;

private:
  juce::AsyncUpdater *const fOwner;

public:
  explicit GameDirectoryScanThreadXbox360(juce::AsyncUpdater *owner);

  void run() override;
  void unsafeRun();

private:
  void lookupRoot(juce::File root, std::vector<GameDirectory> &buffer);
  void lookupContent(juce::File content, std::vector<GameDirectory> &buffer);
  void lookupContentChild(juce::File child, std::vector<GameDirectory> &buffer);
};

} // namespace je2be::desktop
