#include <je2be.hpp>

#include "File.h"
#include "Xbox360GameDirectoryScanWorker.h"

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

Xbox360GameDirectoryScanWorker::Xbox360GameDirectoryScanWorker(std::weak_ptr<AsyncUpdaterWith<std::vector<GameDirectory>>> owner) : fOwner(owner), fAbort(false) {}

void Xbox360GameDirectoryScanWorker::run() {
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

void Xbox360GameDirectoryScanWorker::unsafeRun() {
  Array<File> roots;
  File::findFileSystemRoots(roots);

  struct Comparator {
    int compareElements(File first, File second) {
      auto a = Order(first);
      auto b = Order(second);
      if (a == b) {
        return second.getFullPathName().compare(first.getFullPathName());
      } else {
        return a - b;
      }
    }

    static int Order(File const &f) {
      int ret = 0;
      if (f.isOnHardDisk()) {
        ret += (1 << 0);
      }
      if (!f.isOnRemovableDrive()) {
        ret += (1 << 1);
      }
      if (IsRemoteDrive(f)) {
        ret += (1 << 2);
      }
      return ret;
    }
  } comparator;
  roots.sort<Comparator>(comparator, true);

  for (File root : roots) {
    if (threadShouldExit()) {
      break;
    }
    size_t before = fGameDirectories.size();
    lookupRoot(root, fGameDirectories);
    auto owner = fOwner.lock();
    if (!owner) {
      break;
    }
    if (fAbort.load()) {
      break;
    }
    if (before != fGameDirectories.size()) {
      owner->triggerAsyncUpdateWith(fGameDirectories);
    }
  }
}

void Xbox360GameDirectoryScanWorker::lookupRoot(File root, std::vector<GameDirectory> &buffer) {
  auto content = root.getChildFile("Content");
  if (!content.isDirectory()) {
    return;
  }
  lookupContent(content, buffer);
}

void Xbox360GameDirectoryScanWorker::lookupContent(File content, std::vector<GameDirectory> &buffer) {
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

void Xbox360GameDirectoryScanWorker::lookupContentChild(File child, std::vector<GameDirectory> &buffer) {
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

void Xbox360GameDirectoryScanWorker::signalThreadShouldExit() {
  fAbort = true;
}

bool Xbox360GameDirectoryScanWorker::threadShouldExit() const {
  return fAbort.load();
}

} // namespace je2be::desktop
