#include "GameDirectoryScanThreadBedrock.h"

using namespace juce;

namespace je2be::gui {

GameDirectoryScanThreadBedrock::GameDirectoryScanThreadBedrock(AsyncUpdater *owner) : Thread("je2be::gui::GameDirectoryScanThreadBedrock"), fOwner(owner) {}

void GameDirectoryScanThreadBedrock::run() {
  try {
    unsafeRun();
    fOwner->triggerAsyncUpdate();
  } catch (...) {
  }
}

void GameDirectoryScanThreadBedrock::unsafeRun() {
  File dir = GameDirectory::BedrockSaveDirectory();
  auto directories = dir.findChildFiles(File::findDirectories, false);
  for (File const &directory : directories) {
    File db = directory.getChildFile("db");
    if (threadShouldExit()) {
      break;
    }
    if (!db.isDirectory()) {
      continue;
    }
    File level = directory.getChildFile("level.dat");
    if (!level.existsAsFile()) {
      continue;
    }
    String levelName = directory.getFileName();
    File levelNameFile = directory.getChildFile("levelname.txt");
    if (levelNameFile.existsAsFile()) {
      StringArray lines;
      levelNameFile.readLines(lines);
      if (!lines.isEmpty()) {
        levelName = lines[0];
      }
    }
    File worldIconFile = directory.getChildFile("world_icon.jpeg");
    Image worldIcon;
    if (worldIconFile.existsAsFile()) {
      worldIcon = JPEGImageFormat::loadFrom(worldIconFile);
    }
    GameDirectory gd;
    gd.fDirectory = directory;
    gd.fLevelName = levelName;
    gd.fIcon = worldIcon;
    fGameDirectories.push_back(gd);
  }
}

} // namespace je2be::gui
