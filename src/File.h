#pragma once

#include "Status.hpp"
#include <juce_gui_extra/juce_gui_extra.h>

#if defined(CopyFile)
#undef CopyFile
#endif

namespace je2be::desktop {

static inline std::filesystem::path PathFromFile(juce::File file) {
#if defined(_WIN32)
  return std::filesystem::path(file.getFullPathName().toWideCharPointer());
#else
  return std::filesystem::path(file.getFullPathName().toStdString());
#endif
}

static std::string PathStringForLogging(juce::File file) {
#define TYPE(__name) \
  { juce::File::__name, "(" #__name ")" }
  static std::unordered_map<juce::File::SpecialLocationType, std::string> const sTypes = {
      TYPE(userHomeDirectory),
      TYPE(userDocumentsDirectory),
      TYPE(userDesktopDirectory),
      TYPE(userMusicDirectory),
      TYPE(userMoviesDirectory),
      TYPE(userPicturesDirectory),
      TYPE(userApplicationDataDirectory),
      TYPE(commonApplicationDataDirectory),
      TYPE(commonDocumentsDirectory),
      TYPE(tempDirectory),
      TYPE(currentExecutableFile),
      TYPE(currentApplicationFile),
      TYPE(invokedExecutableFile),
      TYPE(hostApplicationPath),
      TYPE(windowsSystemDirectory),
      TYPE(globalApplicationsDirectory),
      TYPE(globalApplicationsDirectoryX86),
      TYPE(windowsLocalAppData),
  };
#undef TYPE

  juce::String full = file.getFullPathName();
  juce::String longestMatch;
  std::string candidate = full.toStdString();
  for (auto const &type : sTypes) {
    auto location = juce::File::getSpecialLocation(type.first);
    if (file.isAChildOf(location)) {
      auto locationFull = location.getFullPathName();
      if (locationFull.length() > longestMatch.length()) {
        longestMatch = locationFull;
        candidate = type.second + file.getRelativePathFrom(location).toStdString();
      }
    }
  }
  return candidate;
}

static Status CopyFile(juce::File from, juce::File to, char const *sourceFileLocation, int sourceFileLine) {
  if (from.copyFileTo(to)) {
    return Status::Ok();
  }
  return Error(sourceFileLocation, sourceFileLine, "failed copying file from " + PathStringForLogging(from) + " to " + PathStringForLogging(to));
}

static Status CopyDirectoryRecursive(juce::File from, juce::File to, std::vector<juce::File> const &exclude) {
  using namespace juce;
  if (from.isDirectory()) {
    if (auto st = to.createDirectory(); !st.ok()) {
      return Error(__FILE__, __LINE__, st.getErrorMessage().toStdString());
    }
  }
  for (File &file : from.findChildFiles(File::findFiles, false)) {
    bool excluding = false;
    for (File const &e : exclude) {
      if (file == e) {
        excluding = true;
        break;
      }
    }
    if (excluding) {
      continue;
    }
    File copyTo = to.getChildFile(file.getFileName());
    if (auto st = CopyFile(file, copyTo, __FILE__, __LINE__); !st.ok()) {
      return st;
    }
  }
  for (File &dir : from.findChildFiles(File::findDirectories, false)) {
    bool excluding = false;
    for (File const &e : exclude) {
      if (dir == e) {
        excluding = true;
        break;
      }
    }
    if (excluding) {
      continue;
    }
    if (auto st = CopyDirectoryRecursive(dir, to.getChildFile(dir.getFileName()), exclude); !st.ok()) {
      return st;
    }
  }
  return Status::Ok();
}

} // namespace je2be::desktop
