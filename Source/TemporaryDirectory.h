#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace je2be::gui {

class TemporaryDirectory {
public:
  static juce::File EnsureExisting() {
    juce::File current = Get();
    if (current.exists()) {
      return current;
    }
    current.createDirectory();
    return current;
  }

  static void CleanupAsync() {
    juce::File current = Get();
    juce::File root = AppTempRootDir();

    std::thread th(
        [](juce::File root, juce::File current) {
          for (auto it : juce::RangedDirectoryIterator(root, false, "*", juce::File::findFilesAndDirectories)) {
            juce::File file = it.getFile();
            if (file == current)
              continue;
            file.deleteRecursively();
          }
        },
        root, current);
    th.detach();
  }

private:
  static juce::File Get() {
    static juce::File const t(Create());
    return t;
  }

  static juce::File AppTempRootDir() {
    juce::File systemTemp = juce::File::getSpecialLocation(juce::File::tempDirectory);
    return systemTemp.getChildFile("je2be");
  }

  static juce::File Create() {
    juce::Uuid u;
    return AppTempRootDir().getChildFile(u.toDashedString());
  }

private:
  TemporaryDirectory() = delete;
};

} // namespace je2be::gui
