#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace je2be::gui {

struct GameDirectory {
  juce::File fDirectory;
  juce::String fLevelName;
  juce::Image fIcon;

  void paint(juce::Graphics &g, int width, int height, bool selected, juce::Component &component) const {
    using namespace juce;

    Colour hilightColour = component.findColour(DirectoryContentsDisplayComponent::highlightColourId);
    if (selected) {
      g.fillAll(hilightColour);
    }

    int margin = 5;
    int iconSize = height - 2 * margin;
    Rectangle<int> iconBounds(margin, margin, iconSize, iconSize);
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
    g.drawFittedText(fDirectory.getFileName(), x, y, lineWidth, lineHeight, Justification::centredLeft, 1);
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
};

} // namespace je2be::gui
