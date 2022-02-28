#include <je2be.hpp>

#include "File.h"
#include "GameDirectoryScanThreadJava.h"

using namespace juce;

namespace je2be::gui {

GameDirectoryScanThreadJava::GameDirectoryScanThreadJava(AsyncUpdater *owner) : Thread("je2be::gui::GameDirectoryScanThreadJava"), fOwner(owner) {}

void GameDirectoryScanThreadJava::run() {
  try {
    unsafeRun();
  } catch (...) {
  }
  fOwner->triggerAsyncUpdate();
}

void GameDirectoryScanThreadJava::unsafeRun() {
  File dir = GameDirectory::JavaSaveDirectory();
  auto directories = dir.findChildFiles(File::findDirectories, false);
  for (File const &directory : directories) {
    if (threadShouldExit()) {
      break;
    }

    File level = directory.getChildFile("level.dat");
    if (!level.existsAsFile()) {
      continue;
    }
    juce::String levelName;
    Time lastUpdate;
    GameDirectory::GameMode mode = GameDirectory::GameMode::SURVIVAL;
    juce::String version;
    bool commandsEnabled = false;
    auto s = std::make_shared<mcfile::stream::GzFileInputStream>(PathFromFile(level));
    if (auto tag = CompoundTag::Read(s, std::endian::big); tag) {
      if (auto data = tag->compoundTag("Data"); data) {
        if (auto lastPlayed = data->int64("LastPlayed"); lastPlayed) {
          lastUpdate = Time(*lastPlayed);
        }
        if (auto gameType = data->int32("GameType"); gameType) {
          mode = static_cast<GameDirectory::GameMode>(*gameType);
        }
        if (auto versionTag = data->compoundTag("Version"); versionTag) {
          if (auto versionName = versionTag->string("Name"); versionName) {
            version = *versionName;
          }
        }
        commandsEnabled = data->boolean("allowCommands", false);
        if (auto levelNameTag = data->string("LevelName"); levelNameTag) {
          levelName = *levelNameTag;
        }
      }
    }

    File worldIconFile = directory.getChildFile("icon.png");
    Image worldIcon;
    if (worldIconFile.existsAsFile()) {
      worldIcon = PNGImageFormat::loadFrom(worldIconFile);
    }

    GameDirectory gd;
    gd.fDirectory = directory;
    gd.fLevelName = levelName;
    gd.fIcon = worldIcon;
    gd.fLastUpdate = lastUpdate;
    gd.fGameMode = mode;
    gd.fVersion = version;
    gd.fCommandsEnabled = commandsEnabled;
    fGameDirectories.push_back(gd);
  }

  std::stable_sort(fGameDirectories.begin(), fGameDirectories.end(), [](GameDirectory const &lhs, GameDirectory const &rhs) {
    return lhs.fLastUpdate > rhs.fLastUpdate;
  });
}

} // namespace je2be::gui
