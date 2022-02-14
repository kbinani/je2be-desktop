#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace je2be::gui {

static inline juce::File BedrockSaveDirectory() {
  return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
      .getParentDirectory()
      .getChildFile("Local")
      .getChildFile("Packages")
      .getChildFile("Microsoft.MinecraftUWP_8wekyb3d8bbwe")
      .getChildFile("LocalState")
      .getChildFile("games")
      .getChildFile("com.mojang")
      .getChildFile("minecraftWorlds");
}

static inline juce::File JavaSaveDirectory() {
  return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile(".minecraft").getChildFile("saves");
}

} // namespace je2be::gui
