#include <je2be.hpp>

#include "File.h"
#include "GameDirectoryScanThreadXbox360.h"

using namespace juce;

namespace je2be::gui {

GameDirectoryScanThreadXbox360::GameDirectoryScanThreadXbox360(AsyncUpdater *owner) : Thread("je2be::gui::GameDirectoryScanThreadXbox360"), fOwner(owner) {}

void GameDirectoryScanThreadXbox360::run() {
  try {
    unsafeRun();
  } catch (...) {
  }
  fOwner->triggerAsyncUpdate();
}

void GameDirectoryScanThreadXbox360::unsafeRun() {
  Array<File> roots;
  File::findFileSystemRoots(roots);
  // TODO:
}

} // namespace je2be::gui
