#include "component/About.h"
#include "BinaryData.h"
#include "component/DrawableText.h"

using namespace juce;

namespace {

double const kScrollSpeedPixelPerSec = 40;
int const kLineHeight = 14;
int const kScrollResetGapMilliSeconds = 3000;

class HeaderComponent : public juce::Component {
public:
  void paint(Graphics &g) override {
    auto color = findColour(ResizableWindow::backgroundColourId);
    g.setColour(color);
    g.fillRect(0, 0, getWidth(), getHeight());
  }
};

} // namespace

namespace je2be::desktop::component {

About::About() {
  std::vector<juce::String> lines = {
      "",
      "Copyright (C) 2020-2024 kbinani",
      "",
      "This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.",
      "",
      "This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License",
      "for more details.",
      "",
      "You should have received a copy of the GNU General Public License along with this program. If not, see <http://www.gnu.org/licenses/>.",
      "",
      "",
      "",
      "This program is not an official Minecraft product.",
      "",
      "Xbox and Xbox 360 are trademarks of the Microsoft group of companies.",
      "",
      "PS3 is a registered trademark of Sony Interactive Entertainment Inc.",
      "",
      "",
      "",
      "Acknowledgement",
      "",
      "Reinforced by PVS-Studio - static analyzer for C, C++, C#, and Java code.",
      "https://pvs-studio.com/en/pvs-studio",
      "",

      "JUCE",
      "https://github.com/juce-framework/JUCE",
      "",

      "json",
      "https://github.com/nlohmann/json",
      "",

      "zlib",
      "https://github.com/madler/zlib",
      "",

      "minizip-ng",
      "https://github.com/zlib-ng/minizip-ng",
      "",

      "libdeflate",
      "https://github.com/ebiggers/libdeflate",
      "",

      "Velocity",
      "https://github.com/hetelek/Velocity",
      "https://github.com/Gualdimar/Velocity",
      "",

      "libmspack",
      "https://www.cabextract.org.uk/libmspack",
      "",

      "XBox to PC Minecraft converter",
      "https://sourceforge.net/projects/xboxtopcminecraftconverter",
      "",

      "mimalloc",
      "https://github.com/microsoft/mimalloc",
      "",

      "sparse",
      "https://github.com/Arzaghi/sparse",
      "",

      "lz4",
      "https://github.com/lz4/lz4",
      "",

      "icu",
      "https://icu.unicode.org",
      "",

      "icu-cmake",
      "https://github.com/AiMiDi/icu-cmake",
      "",

      "defer",
      "https://github.com/kbinani/defer",
      "",

      "LevelDB",
      "https://github.com/kbinani/leveldb-stdfs",
      "",

      "libminecraft-file",
      "https://github.com/kbinani/libminecraft-file",
      "",

      "je2be-core",
      "https://github.com/kbinani/je2be-core",
      "",
  };

  int width = 400;
  int height = 500;
  setSize(width, height);

  fScrollContents.reset(new Component());
  fScrollContents->setOpaque(false);
  fScrollContents->setInterceptsMouseClicks(false, false);

  fHeader.reset(new HeaderComponent());
  fHeader->setInterceptsMouseClicks(false, false);

  int margin = 10;
  int const cwidth = width - 2 * margin;
  int y = margin;
  {
    int logoHeight = 120;
    fLogoComponent.reset(new ImageComponent());
    auto logo = PNGImageFormat::loadFrom(BinaryData::iconlarge_png, BinaryData::iconlarge_pngSize);
    fLogoComponent->setImage(logo);
    fLogoComponent->setBounds(margin, y, cwidth, logoHeight);
    fHeader->addAndMakeVisible(*fLogoComponent);
    y += logoHeight;
  }
  {
    y += margin;
    fAppName.reset(new DrawableText(JUCE_APPLICATION_NAME_STRING, 40));
    fAppName->setBounds(margin, y, cwidth, 0);
    fAppName->shrinkToFit();
    fHeader->addAndMakeVisible(*fAppName);
    y += fAppName->getHeight();
  }
  {
    String version = String("Version: ") + String::fromUTF8(JUCE_APPLICATION_VERSION_STRING);
    fAppVersion.reset(new DrawableText(version, kLineHeight));
    fAppVersion->setBounds(margin, y, cwidth, 0);
    fAppVersion->shrinkToFit();
    fHeader->addAndMakeVisible(*fAppVersion);
    y += fAppVersion->getHeight();
  }
  y += margin / 2;
  int const scrollerTop = y;
  y = margin / 2;
  {
    for (juce::String const &line : lines) {
      auto label = std::make_shared<DrawableText>(line, kLineHeight);
      label->setBounds(margin, y, cwidth, 0);
      label->shrinkToFit();
      fScrollContents->addAndMakeVisible(*label);
      fParagraphs.push_back(label);
      y += label->getHeight();
    }
    y += margin;
  }

  addAndMakeVisible(*fScrollContents);
  fScrollContents->setBounds(0, scrollerTop, width, y);

  fHeader->setBounds(0, 0, width, scrollerTop);
  addAndMakeVisible(*fHeader);

  fAnimator.reset(new ComponentAnimator());
  fNextScrollStartY = scrollerTop;
  fNextStartSpeed = 0;
  startTimer(kScrollResetGapMilliSeconds);
}

void About::timerCallback() {
  int duration = startScrollFrom(fNextScrollStartY, fNextStartSpeed);
  fNextStartSpeed = 1.0;
  stopTimer();
  startTimer(duration);
}

void About::mouseDown(MouseEvent const &e) {
  if (!e.mods.isLeftButtonDown()) {
    return;
  }
  bool animating = fAnimator->isAnimating();
  fAnimator->cancelAnimation(fScrollContents.get(), false);
  stopTimer();
  if (!animating) {
    int currentY = fScrollContents->getY();
    int duration = startScrollFrom(currentY, 1.0);
    startTimer(duration + kScrollResetGapMilliSeconds);
  }
}

int About::startScrollFrom(int startY, double startSpeed) {
  int scrollerTop = fHeader->getBottom();
  int finalY = scrollerTop - fScrollContents->getHeight();
  Rectangle<int> finalBounds(0, finalY, fScrollContents->getWidth(), fScrollContents->getHeight());
  int duration = (startY - finalY) * 1000 / kScrollSpeedPixelPerSec;
  fScrollContents->setBounds(0, startY, fScrollContents->getWidth(), fScrollContents->getHeight());
  fAnimator->animateComponent(fScrollContents.get(), finalBounds, 1, duration, false, startSpeed, 1.0);
  fNextScrollStartY = getHeight();
  return duration;
}

} // namespace je2be::desktop::component
