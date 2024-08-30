#include <je2be.hpp>

#include "File.h"
#include "PS3GameDirectoryScanWorker.h"

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

PS3GameDirectoryScanWorker::PS3GameDirectoryScanWorker(std::weak_ptr<AsyncUpdaterWith<std::vector<GameDirectory>>> owner) : fOwner(owner), fAbort(false) {}

void PS3GameDirectoryScanWorker::run() {
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

void PS3GameDirectoryScanWorker::unsafeRun() {
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

void PS3GameDirectoryScanWorker::lookupRoot(File root, std::vector<GameDirectory> &buffer) {
  auto ps3 = root.getChildFile("PS3");
  if (!ps3.isDirectory()) {
    return;
  }
  lookupPS3(ps3, buffer);
}

void PS3GameDirectoryScanWorker::lookupPS3(File ps3, std::vector<GameDirectory> &buffer) {
  auto savedata = ps3.getChildFile("SAVEDATA");
  if (!savedata.isDirectory()) {
    return;
  }
  lookupSAVEDATA(savedata, buffer);
}

void PS3GameDirectoryScanWorker::lookupSAVEDATA(File savedata, std::vector<GameDirectory> &buffer) {
  auto children = savedata.findChildFiles(File::findDirectories, false, "*", File::FollowSymlinks::no);
  for (auto const &child : children) {
    if (threadShouldExit()) {
      break;
    }
    auto gamedata = child.getChildFile("GAMEDATA");
    if (!gamedata.existsAsFile()) {
      continue;
    }
    GameDirectory gd;
    gd.fDirectory = child;
    // TODO: taking level name from somewhere
    gd.fLevelName = "world";
    auto icon = child.getChildFile("ICON0.PNG");
    if (icon.existsAsFile()) {
      gd.fIcon = PNGImageFormat::loadFrom(icon);
    }
    buffer.push_back(gd);
  }
}

void PS3GameDirectoryScanWorker::signalThreadShouldExit() {
  fAbort = true;
}

bool PS3GameDirectoryScanWorker::threadShouldExit() const {
  return fAbort.load();
}

} // namespace je2be::desktop
