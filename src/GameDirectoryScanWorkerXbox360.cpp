#include <je2be.hpp>

#include "File.h"
#include "GameDirectoryScanWorkerXbox360.h"

using namespace juce;

namespace je2be::desktop {

static bool StringContainsOnlyAlnum(juce::String const &s) {
  if (s.isEmpty()) {
    return false;
  }
  for (int i = 0; i < s.length(); i++) {
    int ch = s[i];
    if (isalnum(ch) == 0) {
      return false;
    }
  }
  return true;
}

GameDirectoryScanWorkerXbox360::GameDirectoryScanWorkerXbox360(std::weak_ptr<AsyncUpdaterWith<std::vector<GameDirectory>>> owner) : fOwner(owner), fAbort(false) {}

void GameDirectoryScanWorkerXbox360::run() {
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

void GameDirectoryScanWorkerXbox360::unsafeRun() {
  Array<File> roots;
  File::findFileSystemRoots(roots);
  for (File root : roots) {
    if (threadShouldExit()) {
      break;
    }
    lookupRoot(root, fGameDirectories);
  }
}

void GameDirectoryScanWorkerXbox360::lookupRoot(File root, std::vector<GameDirectory> &buffer) {
  auto content = root.getChildFile("Content");
  if (!content.isDirectory()) {
    return;
  }
  lookupContent(content, buffer);
}

void GameDirectoryScanWorkerXbox360::lookupContent(File content, std::vector<GameDirectory> &buffer) {
  auto maybeUserProfileIds = content.findChildFiles(File::findDirectories, false, "*", File::FollowSymlinks::no);
  for (auto const &maybeUserProfileId : maybeUserProfileIds) {
    if (threadShouldExit()) {
      break;
    }
    auto name = maybeUserProfileId.getFileName();
    if (name.length() != 16) {
      continue;
    }
    if (!StringContainsOnlyAlnum(name)) {
      continue;
    }
    lookupContentChild(maybeUserProfileId, buffer);
  }
}

void GameDirectoryScanWorkerXbox360::lookupContentChild(File child, std::vector<GameDirectory> &buffer) {
  auto maybeGameTitleIds = child.findChildFiles(File::findDirectories, false, "*", File::FollowSymlinks::no);
  static juce::String const sMaybeMinecraftGameTitleId("584111F7");
  for (auto const &maybeGameTitleId : maybeGameTitleIds) {
    if (threadShouldExit()) {
      break;
    }
    auto name = maybeGameTitleId.getFileName();
    if (name.compareIgnoreCase(sMaybeMinecraftGameTitleId) != 0) {
      continue;
    }
    auto children = maybeGameTitleId.findChildFiles(File::findDirectories, false, "*", File::FollowSymlinks::no);
    for (auto const &child : children) {
      if (threadShouldExit()) {
        break;
      }
      auto childName = child.getFileName();
      if (childName.length() != 8) {
        continue;
      }
      auto saveInfo = child.getChildFile("_MinecraftSaveInfo");
      if (!saveInfo.existsAsFile()) {
        continue;
      }

      std::vector<je2be::box360::MinecraftSaveInfo::SaveBin> bins;
      je2be::box360::MinecraftSaveInfo::Parse(PathFromFile(saveInfo), bins);
      if (bins.empty()) {
        continue;
      }
      for (auto const &bin : bins) {
        if (threadShouldExit()) {
          break;
        }
        GameDirectory gd;
        gd.fDirectory = child.getChildFile(bin.fFileName);
        std::u16string const &title = bin.fTitle;
        gd.fLevelName = juce::String(CharPointer_UTF16((CharPointer_UTF16::CharType const *)title.c_str()), title.size());
        gd.fIcon = PNGImageFormat::loadFrom(bin.fThumbnailData.c_str(), bin.fThumbnailData.size());
        buffer.push_back(gd);
      }
    }
  }
}

void GameDirectoryScanWorkerXbox360::signalThreadShouldExit() {
  fAbort = true;
}

bool GameDirectoryScanWorkerXbox360::threadShouldExit() const {
  return fAbort.load();
}

} // namespace je2be::desktop
