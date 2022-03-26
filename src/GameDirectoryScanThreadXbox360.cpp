#include <je2be.hpp>

#include "File.h"
#include "GameDirectoryScanThreadXbox360.h"

using namespace juce;

namespace je2be::gui {

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

static void LookupContentChild(File child, std::vector<GameDirectory> &buffer) {
  auto maybeGameTitleIds = child.findChildFiles(File::findDirectories, false, "*", File::FollowSymlinks::no);
  for (auto const &maybeGameTitleId : maybeGameTitleIds) {
    auto name = maybeGameTitleId.getFileName();
    if (name.length() != 8) {
      continue;
    }
    if (!StringContainsOnlyAlnum(name)) {
      continue;
    }
    auto children = maybeGameTitleId.findChildFiles(File::findDirectories, false, "*", File::FollowSymlinks::no);
    for (auto const &child : children) {
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

static void LookupContent(File content, std::vector<GameDirectory> &buffer) {
  auto maybeUserProfileIds = content.findChildFiles(File::findDirectories, false, "*", File::FollowSymlinks::no);
  for (auto const &maybeUserProfileId : maybeUserProfileIds) {
    auto name = maybeUserProfileId.getFileName();
    if (name.length() != 16) {
      continue;
    }
    if (!StringContainsOnlyAlnum(name)) {
      continue;
    }
    LookupContentChild(maybeUserProfileId, buffer);
  }
}

static void LookupRoot(File root, std::vector<GameDirectory> &buffer) {
  auto content = root.getChildFile("Content");
  if (!content.isDirectory()) {
    return;
  }
  LookupContent(content, buffer);
}

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
  for (File root : roots) {
    LookupRoot(root, fGameDirectories);
  }
}

} // namespace je2be::gui
