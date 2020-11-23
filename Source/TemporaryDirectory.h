#pragma once

#include "JuceHeader.h"

class TemporaryDirectory {
public:
  static File EnsureExisting() {
    File current = Get();
    if (current.exists()) {
      return current;
    }
    current.createDirectory();
    return current;
  }

  static void CleanupAsync() {
    File current = Get();
    File root = AppTempRootDir();

    std::thread th(
        [](File root, File current) {
          for (auto it : RangedDirectoryIterator(
                   root, false, "*", File::findFilesAndDirectories)) {
            File file = it.getFile();
            if (file == current)
              continue;
            file.deleteRecursively();
          }
        },
        root, current);
    th.detach();
  }

private:
  static File Get() {
    static File const t(Create());
    return t;
  }

  static File AppTempRootDir() {
    File systemTemp = File::getSpecialLocation(File::tempDirectory);
    return systemTemp.getChildFile("je2be");
  }

  static File Create() {
    Uuid u;
    return AppTempRootDir().getChildFile(u.toDashedString());
  }

private:
  TemporaryDirectory() = delete;
};
