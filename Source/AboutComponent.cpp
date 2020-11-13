#include "AboutComponent.h"
#include "BinaryData.h"

static Component *createLabel(String t, int height = 14) {
  Label *l = new Label();
  l->setText(t, NotificationType::dontSendNotification);
  l->setBounds(0, 0, 100, height);
  l->setColour(Label::textColourId, Colours::white);
  l->setJustificationType(Justification::centred);
  return l;
}

AboutComponent::AboutComponent() {
  fLines = {
      createLabel(String("Version: ") +
                  String::fromUTF8(ProjectInfo::versionString)),
      createLabel("Copyright (C) 2020 kbinani"),
      createLabel(""),
      createLabel("Acknowledgement"),
      createLabel(""),

      createLabel("JUCE"),
      createLabel("https://github.com/juce-framework/JUCE"),
      createLabel(""),

      createLabel("je2be (core library)"),
      createLabel("https://github.com/kbinani/je2be"),
      createLabel(""),

      createLabel("libminecraft-file"),
      createLabel("https://github.com/kbinani/libminecraft-file"),
      createLabel(""),
  };
  for (auto const &line : fLines) {
    addAndMakeVisible(*line);
  }
  fLogo = Drawable::createFromImageData(BinaryData::iconlarge_png,
                                        BinaryData::iconlarge_pngSize);
  setSize(400, 500);
}

void AboutComponent::paint(Graphics &g) {
  g.saveState();

  int const margin = 10;
  int const width = getWidth();
  float y = margin;

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
  for (auto const &line : fLines) {
    line->setBounds(0, y, width, line->getHeight());
    y += line->getHeight();
  }
  g.restoreState();
}
