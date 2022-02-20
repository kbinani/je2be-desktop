#pragma once

namespace je2be::gui {

static inline std::filesystem::path PathFromFile(juce::File file) {
#if defined(_WIN32)
  return std::filesystem::path(file.getFullPathName().toWideCharPointer());
#else
  return std::filesystem::path(file.getFullPathName().toStdString());
#endif
}

static bool CopyDirectoryRecursive(juce::File from, juce::File to, std::vector<juce::File> const &exclude) {
  using namespace juce;
  if (from.isDirectory() && !to.createDirectory()) {
    return false;
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
    if (!file.copyFileTo(to.getChildFile(file.getFileName()))) {
      return false;
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
    if (!CopyDirectoryRecursive(dir, to.getChildFile(dir.getFileName()), exclude)) {
      return false;
    }
  }
  return true;
}

} // namespace je2be::gui
