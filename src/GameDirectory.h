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

  void paint(juce::Graphics &g, int width, int height, bool selected, juce::Component &component, juce::String const &search) const {
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

    Font font = g.getCurrentFont().withHeight(15);

    int const x = iconBounds.getRight() + margin;
    Colour textColour;
    if (selected) {
      textColour = component.findColour(DirectoryContentsDisplayComponent::highlightedTextColourId);
    } else {
      textColour = component.findColour(DirectoryContentsDisplayComponent::textColourId);
    }
    juce::AttributedString levelName = DecorateMatches(fLevelName, search, textColour, Colours::red);
    levelName.setFont(font);
    levelName.setJustification(Justification::centredLeft);
    levelName.setWordWrap(juce::AttributedString::none);
    int lineHeight = (height - 2 * margin) / 3;
    int lineWidth = width - x - margin;
    int y = margin;
    float stringWidth = MeasureStringWidth(font, fLevelName);
    if (stringWidth > lineWidth) {
      g.saveState();
      float scale = lineWidth / stringWidth;
      g.addTransform(juce::AffineTransform::translation(x, y));
      g.addTransform(juce::AffineTransform::scale(scale, 1));
      levelName.draw(g, juce::Rectangle<float>(0, 0, 1e8, lineHeight));
      g.restoreState();
    } else {
      levelName.draw(g, juce::Rectangle<float>(x, y, 1e8, lineHeight));
    }

    y += lineHeight;
    juce::String secondLine = fDirectory.getFileName();
    if (fLastUpdate) {
      secondLine += " (" + StringFromTime(*fLastUpdate) + ")";
    }
    AttributedString directoryName = DecorateMatches(secondLine, search, textColour.darker(0.8), Colours::yellow);
    directoryName.setFont(font);
    directoryName.setWordWrap(juce::AttributedString::none);
    directoryName.setJustification(Justification::centredLeft);
    stringWidth = MeasureStringWidth(font, secondLine);
    if (stringWidth > lineWidth) {
      g.saveState();
      float scale = lineWidth / stringWidth;
      g.addTransform(juce::AffineTransform::translation(x, y));
      g.addTransform(juce::AffineTransform::scale(scale, 1));
      directoryName.draw(g, juce::Rectangle<float>(0, 0, 1e8, lineHeight));
      g.restoreState();
    } else {
      directoryName.draw(g, juce::Rectangle<float>(x, y, 1e8, lineHeight));
    }

    y += lineHeight;
    g.setColour(textColour.darker(0.8));
    g.setFont(15);
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

  static float MeasureStringWidth(juce::Font font, juce::String const &s) {
    return font.getStringWidthFloat(s);
  }

  static juce::AttributedString DecorateMatches(juce::String s, juce::String const &search, juce::Colour base, juce::Colour highlight) {
    juce::AttributedString ret;
    int idx = s.indexOfIgnoreCase(search);
    while (idx >= 0 && search.isNotEmpty()) {
      auto sub = s.substring(0, idx);
      juce::AttributedString part1;
      part1.setText(sub);
      part1.setColour(base);
      ret.append(part1);
      auto match = s.substring(idx, idx + search.length());
      juce::AttributedString part2;
      part2.setText(match);
      part2.setColour(highlight);
      ret.append(part2);
      s = s.substring(idx + search.length());
      idx = s.indexOfIgnoreCase(search);
    }
    juce::AttributedString last;
    last.setText(s);
    last.setColour(base);
    ret.append(last);
    return ret;
  }

  bool match(juce::String const &search) const {
    if (search.isEmpty()) {
      return true;
    }
    if (fLevelName.indexOfIgnoreCase(search) >= 0) {
      return true;
    }
    return fDirectory.getFileName().indexOfIgnoreCase(search) >= 0;
  }

  static juce::File BedrockSaveDirectory() {
    auto userApplicationDataDirectory = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    auto users = userApplicationDataDirectory.getChildFile("Minecraft Bedrock").getChildFile("Users");
    if (users.exists() && users.isDirectory()) {
      juce::DirectoryIterator itr(users, false, "*", juce::File::findDirectories);
      juce::String userId;
      juce::String const digits = "0123456789";
      while (itr.next()) {
        auto sub = itr.getFile();
        auto n = sub.getFileName();
        if (n == u8"Shared") {
          continue;
        }
        if (n.startsWith("-")) {
          // Not sure if negative userId exists
          if (n.length() == 1) {
            continue;
          }
          if (!n.substring(1).containsOnly(digits)) {
            continue;
          }
        } else {
          if (!n.containsOnly(digits)) {
            continue;
          }
        }
        auto moj = sub.getChildFile("games").getChildFile("com.mojang");
        if (!moj.exists() || !moj.isDirectory()) {
          continue;
        }
        userId = n;
        break;
      }
      if (userId.isNotEmpty()) {
        return users.getChildFile(userId).getChildFile("games").getChildFile("com.mojang").getChildFile("minecraftWorlds");
      }
    }
    return userApplicationDataDirectory
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
