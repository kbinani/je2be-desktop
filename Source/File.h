#pragma once

namespace je2be::gui {

static inline std::filesystem::path PathFromFile(juce::File file) {
#if defined(_WIN32)
  return std::filesystem::path(file.getFullPathName().toWideCharPointer());
#else
  return std::filesystem::path(file.getFullPathName().toStdString());
#endif
}

} // namespace je2be::gui
