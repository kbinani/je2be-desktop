#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace je2be::desktop {

struct GameDirectory {
  juce::File fDirectory;
  juce::String fLevelName;
  juce::Image fIcon;
  std::optional<juce::Time> fLastUpdate;

  enum GameMode : int32_t {
    SURVIVAL = 0,
    CREATIVE = 1,
    ADVENTURE = 2,
    SPECTATOR = 3,
  };
  std::optional<GameMode> fGameMode;

  std::optional<juce::String> fVersion;
  std::optional<bool> fCommandsEnabled;

  void paint(juce::Graphics &g, int width, int height, bool selected, juce::Component &component) const {
    using namespace juce;

    Colour hilightColour = component.findColour(DirectoryContentsDisplayComponent::highlightColourId);
    if (selected) {
      g.fillAll(hilightColour);
    }

    int margin = 5;
    int iconSize = height - 2 * margin;
    juce::Rectangle<int> iconBounds(margin, margin, iconSize, iconSize);
    if (fIcon.isValid()) {
      int iconWidth = iconSize * fIcon.getWidth() / fIcon.getHeight();
      iconBounds.setWidth(iconWidth);
      g.drawImageWithin(fIcon, iconBounds.getX(), iconBounds.getY(), iconBounds.getWidth(), iconBounds.getHeight(), RectanglePlacement::onlyReduceInSize);
    }
    g.setColour(hilightColour);
    g.drawRect(iconBounds);

    int const x = iconBounds.getRight() + margin;
    Colour textColour;
    if (selected) {
      textColour = component.findColour(DirectoryContentsDisplayComponent::highlightedTextColourId);
    } else {
      textColour = component.findColour(DirectoryContentsDisplayComponent::textColourId);
    }
    g.setColour(textColour);
    g.setFont(15);
    int lineHeight = (height - 2 * margin) / 3;
    int lineWidth = width - x - margin;
    int y = margin;
    g.drawFittedText(fLevelName, x, y, lineWidth, lineHeight, Justification::centredLeft, 1);

    y += lineHeight;
    g.setColour(textColour.darker(0.8));
    juce::String secondLine = fDirectory.getFileName();
    if (fLastUpdate) {
      secondLine += " (" + StringFromTime(*fLastUpdate) + ")";
    }
    g.drawFittedText(secondLine, x, y, lineWidth, lineHeight, Justification::centredLeft, 1);

    y += lineHeight;
    if (fGameMode && fCommandsEnabled && fVersion) {
      juce::String thirdLine = StringFromGameMode(*fGameMode) + ", ";
      if (*fCommandsEnabled) {
        thirdLine += TRANS("Cheats") + ", ";
      }
      thirdLine += TRANS("Version") + ": " + *fVersion;
      g.drawFittedText(thirdLine, x, y, lineWidth, lineHeight, Justification::centredLeft, 1);
    } else {
      juce::String thirdLine = fDirectory.getParentDirectory().getFullPathName();
      g.drawFittedText(thirdLine, x, y, lineWidth, lineHeight, Justification::centredLeft, 1);
    }
  }

  static juce::File BedrockSaveDirectory() {
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

  static juce::File JavaSaveDirectory() {
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile(".minecraft").getChildFile("saves");
  }

  static juce::String StringFromTime(juce::Time const &t) {
    int year = t.getYear();
    int month = t.getMonth() + 1;
    int day = t.getDayOfMonth();
    int hour = t.getHours();
    int minute = t.getMinutes();
    return juce::String(year) + "/" + juce::String::formatted("%02d", month) + "/" + juce::String::formatted("%02d", day) + " " + juce::String(hour) + ":" + juce::String::formatted("%02d", minute);
  }

  static juce::String StringFromGameMode(GameMode m) {
    switch (m) {
    case CREATIVE:
      return TRANS("Creative Mode");
    case ADVENTURE:
      return TRANS("Adventure Mode");
    case SPECTATOR:
      return TRANS("Spectator Mode");
    case SURVIVAL:
    default:
      return TRANS("Survival Mode");
    }
  }
};

} // namespace je2be::desktop
