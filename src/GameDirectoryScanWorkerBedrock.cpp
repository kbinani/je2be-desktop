#include <je2be.hpp>

#include "File.h"
#include "GameDirectoryScanWorkerBedrock.h"

using namespace juce;

namespace je2be::desktop {

GameDirectoryScanWorkerBedrock::GameDirectoryScanWorkerBedrock(std::weak_ptr<AsyncUpdaterWith<std::vector<GameDirectory>>> owner) : fOwner(owner), fAbort(false) {}

void GameDirectoryScanWorkerBedrock::run() {
  try {
    unsafeRun();
  } catch (...) {
  }
  if (!fAbort.load()) {
    if (auto worker = fOwner.lock(); worker) {
      worker->triggerAsyncUpdateWith(fGameDirectories);
    }
  }
}

void GameDirectoryScanWorkerBedrock::unsafeRun() {
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
    GameDirectory::GameMode mode = GameDirectory::GameMode::SURVIVAL;
    juce::String version;
    bool commandsEnabled = false;
    auto s = std::make_shared<mcfile::stream::GzFileInputStream>(PathFromFile(level));
    if (s->valid() && s->seek(8)) {
      if (auto tag = CompoundTag::Read(s, mcfile::Endian::Little); tag) {
        if (auto lastPlayed = tag->int64(u8"LastPlayed"); lastPlayed) {
          lastUpdate = Time(*lastPlayed * 1000);
        }
        if (auto gameType = tag->int32(u8"GameType"); gameType) {
          mode = static_cast<GameDirectory::GameMode>(*gameType);
        }
        if (auto lastOpened = tag->listTag(u8"lastOpenedWithVersion"); lastOpened) {
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
        commandsEnabled = tag->boolean(u8"commandsEnabled", false);
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

void GameDirectoryScanWorkerBedrock::signalThreadShouldExit() {
  fAbort = true;
}

bool GameDirectoryScanWorkerBedrock::threadShouldExit() const {
  return fAbort.load();
}

} // namespace je2be::desktop
