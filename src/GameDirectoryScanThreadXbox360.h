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
};

} // namespace je2be::desktop
