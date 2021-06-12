#include "AboutComponent.h"
#include "BinaryData.h"

namespace {
int const kHeaderHeight = 204;
double const kScrollSpeedPixelPerSec = 40;
int const kLineHeight = 14;
double const kSteadySeconds = 3;
int const kTimerHz = 32;
} // namespace

AboutComponent::AboutComponent() {
  fHeaderLines = {
      String("Version: ") + String::fromUTF8(ProjectInfo::versionString),
  };
  fLines = {
      "",
      "Copyright (C) 2020, 2021 kbinani",
      "",
      "This program is free software: you can redistribute it and/or",
      "modify it under the terms of the GNU General Public License as",
      "published by the Free Software Foundation, either version 3 of",
      "the License, or (at your option) any later version.",
      "",
      "This program is distributed in the hope that it will be useful,",
      "but WITHOUT ANY WARRANTY; without even the implied",
      "warranty of MERCHANTABILITY or FITNESS FOR A",
      "PARTICULAR PURPOSE. See the GNU General Public License",
      "for more details.",
      "",
      "You should have received a copy of the GNU General Public License",
      "along with this program.",
      "If not, see <http://www.gnu.org/licenses/>.",
      "",
      "",
      "",
      "Disclaimer",
      "",
      "This program is not an official Minecraft product.",
      ""
      "",
      "",
      "Acknowledgement",
      "",

      "JUCE",
      "https://github.com/juce-framework/JUCE",
      "",

      "LevelDB",
      "https://github.com/google/leveldb",
      "https://github.com/pmmp/leveldb",
      "",

      "xxHash",
      "https://github.com/Cyan4973/xxHash",
      "",

      "json",
      "https://github.com/nlohmann/json",
      "",

      "zlib",
      "https://github.com/madler/zlib",
      "https://github.com/commontk/zlib",
      "",

      "thread-pool",
      "https://github.com/mtrebi/thread-pool",
      "",

      "je2be (core library)",
      "https://github.com/kbinani/je2be",
      "",

      "libminecraft-file",
      "https://github.com/kbinani/libminecraft-file",
      "",
  };
  fLogo = Drawable::createFromImageData(BinaryData::iconlarge_png,
                                        BinaryData::iconlarge_pngSize);
  setSize(400, 500);
  fLastTick = std::chrono::high_resolution_clock::now();
  fScrollDuration = std::chrono::high_resolution_clock::duration(0);
  startTimerHz(kTimerHz);
}

void AboutComponent::timerCallback() {
  auto now = std::chrono::high_resolution_clock::now();
  if (fScrolling) {
    fScrollDuration += now - fLastTick;
  }
  fLastTick = now;
  repaint();
}

void AboutComponent::paint(Graphics &g) {
  Graphics::ScopedSaveState s(g);

  int const margin = 10;
  int const width = getWidth();
  int const height = getHeight();

  float scroll = 0;
  auto sec =
      std::chrono::duration_cast<std::chrono::duration<double>>(fScrollDuration)
          .count();
  if (sec <= kSteadySeconds) {
    scroll = 0;
  } else {
    scroll = (sec - kSteadySeconds) * kScrollSpeedPixelPerSec;
  }
  float y = kHeaderHeight;
  g.setColour(Colours::white);
  int fontSize = kLineHeight;
  g.setFont(fontSize);
  float scrollHeight =
      kLineHeight * (int)fLines.size() + (height - kHeaderHeight);
  for (auto const &line : fLines) {
    float pos = y - kHeaderHeight - scroll;
    int vPos = fmod(fmod(pos, scrollHeight) + scrollHeight, scrollHeight) +
               kHeaderHeight;
    g.drawSingleLineText(line, width / 2, vPos,
                         Justification::horizontallyCentred);
    y += kLineHeight;
  }

  y = margin;

  g.setColour(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
  g.fillRect(0, 0, width, kHeaderHeight);

  {
    int const logoHeight = 120;
    Rectangle<float> logoArea(margin, y, width - 2 * margin, logoHeight);
    fLogo->drawWithin(g, logoArea, RectanglePlacement::centred, 1.0f);
    y += logoHeight + margin;
  }
  {
    int const titleHeight = 40;
    g.setFont(titleHeight);
    g.setColour(Colours::white);
    g.drawText(ProjectInfo::projectName, margin, y, width - 2 * margin,
               titleHeight, Justification::centred);
    y += titleHeight + margin;
  }
  g.setFont(fontSize);
  g.setColour(Colours::white);
  for (auto const &line : fHeaderLines) {
    g.drawSingleLineText(line, width / 2, y,
                         Justification::horizontallyCentred);
    y += kLineHeight;
  }
}

void AboutComponent::mouseDown(MouseEvent const &e) {
  if (!e.mods.isLeftButtonDown()) {
    return;
  }
  auto sec =
      std::chrono::duration_cast<std::chrono::duration<double>>(fScrollDuration)
          .count();
  if (sec <= kSteadySeconds) {
    fScrolling = true;
  } else {
    fScrolling = !fScrolling;
  }
  if (fScrolling != isTimerRunning()) {
    if (fScrolling) {
      fLastTick = std::chrono::high_resolution_clock::now();
      startTimerHz(kTimerHz);
    } else {
      stopTimer();
    }
  }
}
