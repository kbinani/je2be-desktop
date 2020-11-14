#include "AboutComponent.h"
#include "BinaryData.h"

namespace {
int const kHeaderHeight = 232;
double const kScrollSpeedPixelPerSec = 40;
int const kLineHeight = 14;
double const kSteadySeconds = 3;
} // namespace

AboutComponent::AboutComponent() {
  fHeaderLines = {
      String("Version: ") + String::fromUTF8(ProjectInfo::versionString),
      "Copyright (C) 2020 kbinani",
      "",
  };
  fLines = {
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
  fStartTime = std::chrono::high_resolution_clock::now();
  startTimerHz(32);
}

void AboutComponent::timerCallback() { repaint(); }

void AboutComponent::paint(Graphics &g) {
  g.saveState();

  int const margin = 10;
  int const width = getWidth();

  float scroll = 0;
  auto elapsed = std::chrono::high_resolution_clock::now() - fStartTime;
  auto sec = std::chrono::duration_cast<std::chrono::duration<double>>(elapsed)
                 .count();
  if (sec <= kSteadySeconds) {
    scroll = 0;
  } else {
    scroll = (sec - kSteadySeconds) * kScrollSpeedPixelPerSec;
  }
  float y = kHeaderHeight;
  g.setColour(Colours::white);
  float scrollHeight = 1.8 * kLineHeight * fLines.size();
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
    g.saveState();
    g.setFont(titleHeight);
    g.setColour(Colours::white);
    g.drawText(ProjectInfo::projectName, margin, y, width - 2 * margin,
               titleHeight, Justification::centred);
    g.restoreState();
    y += titleHeight + margin;
  }
  g.setColour(Colours::white);
  for (auto const &line : fHeaderLines) {
    g.drawSingleLineText(line, width / 2, y,
                         Justification::horizontallyCentred);
    y += kLineHeight;
  }
  g.restoreState();
}
