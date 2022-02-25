#include <je2be.hpp>

#include "File.h"
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
    Time lastUpdate;
    GameDirectory::GameMode mode;
    juce::String version;
    bool commandsEnabled = false;
    auto s = std::make_shared<mcfile::stream::GzFileInputStream>(PathFromFile(level));
    if (s->valid() && s->seek(8)) {
      if (auto tag = CompoundTag::Read(s, std::endian::little); tag) {
        if (auto lastPlayed = tag->int64("LastPlayed"); lastPlayed) {
          lastUpdate = Time(*lastPlayed * 1000);
        }
        if (auto gameType = tag->int32("GameType"); gameType) {
          mode = static_cast<GameDirectory::GameMode>(*gameType);
        }
        if (auto lastOpened = tag->listTag("lastOpenedWithVersion"); lastOpened) {
          for (auto const &it : *lastOpened) {
            if (auto c = it->asInt(); c) {
              if (version.isEmpty()) {
                version = juce::String(c->fValue);
              } else {
                version += "." + juce::String(c->fValue);
              }
            }
          }
        }
        commandsEnabled = tag->boolean("commandsEnabled", false);
      }
    }

    juce::String levelName = directory.getFileName();
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
