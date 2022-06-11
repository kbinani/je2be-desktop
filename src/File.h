#pragma once

#include "Status.hpp"
#include <juce_gui_extra/juce_gui_extra.h>

namespace je2be::desktop {

static inline std::filesystem::path PathFromFile(juce::File file) {
#if defined(_WIN32)
  return std::filesystem::path(file.getFullPathName().toWideCharPointer());
#else
  return std::filesystem::path(file.getFullPathName().toStdString());
#endif
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
    if (!file.copyFileTo(copyTo)) {
      return Error(__FILE__, __LINE__, "failed copying file from " + file.getFullPathName().toStdString() + " to " + copyTo.getFullPathName().toStdString());
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
