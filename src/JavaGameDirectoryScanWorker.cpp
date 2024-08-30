#include <je2be.hpp>

#include "File.h"
#include "JavaGameDirectoryScanWorker.h"

using namespace juce;

namespace je2be::desktop {

JavaGameDirectoryScanWorker::JavaGameDirectoryScanWorker(std::weak_ptr<AsyncUpdaterWith<std::vector<GameDirectory>>> owner) : fOwner(owner), fAbort(false) {}

void JavaGameDirectoryScanWorker::run() {
  try {
    unsafeRun();
  } catch (...) {
  }
  if (!fAbort.load()) {
    if (auto owner = fOwner.lock(); owner) {
      owner->triggerAsyncUpdateWith(fGameDirectories);
    }
  }
}

void JavaGameDirectoryScanWorker::unsafeRun() {
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
    if (auto tag = CompoundTag::Read(s, mcfile::Encoding::Java); tag) {
      if (auto data = tag->compoundTag(u8"Data"); data) {
        if (auto lastPlayed = data->int64(u8"LastPlayed"); lastPlayed) {
          lastUpdate = Time(*lastPlayed);
        }
        if (auto gameType = data->int32(u8"GameType"); gameType) {
          mode = static_cast<GameDirectory::GameMode>(*gameType);
        }
        if (auto versionTag = data->compoundTag(u8"Version"); versionTag) {
          if (auto versionName = versionTag->string(u8"Name"); versionName) {
            version = juce::String::fromUTF8((char const *)versionName->c_str(), versionName->size());
          }
        }
        commandsEnabled = data->boolean(u8"allowCommands", false);
        if (auto levelNameTag = data->string(u8"LevelName"); levelNameTag) {
          levelName = juce::String::fromUTF8((char const *)levelNameTag->c_str(), levelNameTag->size());
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

void JavaGameDirectoryScanWorker::signalThreadShouldExit() {
  fAbort = true;
}

bool JavaGameDirectoryScanWorker::threadShouldExit() const {
  return fAbort.load();
}

} // namespace je2be::desktop
