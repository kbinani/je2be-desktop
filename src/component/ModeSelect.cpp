#include "component/ModeSelect.h"
#include "CommandID.h"
#include "Constants.h"
#include "component/About.h"
#include "component/TextButton.h"

using namespace juce;

namespace je2be::desktop::component {

namespace {

juce::Path CreateButtonConnectingPath(TextButton const &from, TextButton const &to) {
  juce::Path p;
  auto f = from.getBounds();
  auto t = to.getBounds();
  auto x0 = f.getRight();
  auto y0 = f.getCentreY();
  auto x1 = t.getX();
  auto y1 = t.getCentreY();
  auto span = x1 - x0;
  p.startNewSubPath(x0, y0);
  p.cubicTo(x0 + span * 0.5, y0, x1 - span * 0.5, y1, x1, y1);
  return p;
}

juce::Path CreateArrowHead(TextButton const &from, TextButton const &to) {
  auto f = from.getBounds();
  auto t = to.getBounds();
  auto x0 = f.getRight();
  auto y0 = f.getCentreY();
  auto x1 = t.getX();
  auto y1 = t.getCentreY();
  double length = 30;
  juce::Path p;
  double constexpr radian = juce::degreesToRadians(30.0);
  double angle = atan2(y1 - y0, x1 - x0) * 0.25;
  double dx = length * cos(angle - radian);
  double dy = length * sin(angle - radian);
  p.startNewSubPath(x1 - length * cos(angle - radian), y1 - length * sin(angle - radian));
  p.lineTo(x1, y1);
  p.lineTo(x1 - length * cos(angle + radian), y1 - length * sin(angle + radian));
  return p;
}

} // namespace

ModeSelect::ModeSelect() {
  setSize(kWindowWidth, kWindowHeight);

  int const buttonWidth = kWindowWidth / 4;

  auto &laf = getLookAndFeel();
  laf.setColour(juce::TextButton::ColourIds::buttonOnColourId, laf.findColour(juce::TextButton::ColourIds::textColourOffId));
  laf.setColour(juce::TextButton::ColourIds::textColourOnId, laf.findColour(juce::TextButton::ColourIds::buttonColourId));

  int y = kMargin;
  fLabel.reset(new Label("", TRANS("Select conversion mode") + ":"));
  fLabel->setJustificationType(Justification::centred);
  fLabel->setBounds(kMargin, y, kWindowWidth - 2 * kMargin, kButtonBaseHeight);
  addAndMakeVisible(*fLabel);
  y += fLabel->getHeight();

  y += kMargin;

  fFromJavaButton.reset(new TextButton(TRANS("from Java")));
  fFromJavaButton->setBounds(kWindowWidth / 2 - buttonWidth * 1.5, y, buttonWidth, kButtonBaseHeight);
  fFromJavaButton->onClick = [this]() {
    fFrom = From::Java;
    if (fTo == To::Java) {
      fTo = std::nullopt;
    } else if (fLastTo != To::Java) {
      fTo = fLastTo;
    }
    fLastFrom = From::Java;
    update();
  };
  addAndMakeVisible(*fFromJavaButton);
  y += fFromJavaButton->getHeight();

  y += kMargin;

  fFromBedrockButton.reset(new TextButton(TRANS("from Bedrock")));
  fFromBedrockButton->setBounds(kWindowWidth / 2 - buttonWidth * 1.5, y, buttonWidth, kButtonBaseHeight);
  fFromBedrockButton->onClick = [this]() {
    fFrom = From::Bedrock;
    if (fTo == To::Bedrock) {
      fTo = std::nullopt;
    } else if (fLastTo != To::Bedrock) {
      fTo = fLastTo;
    }
    fLastFrom = From::Bedrock;
    update();
  };
  fFromBedrockButton->setToggleable(true);
  addAndMakeVisible(*fFromBedrockButton);

  fToJavaButton.reset(new TextButton(TRANS("to Java")));
  fToJavaButton->setBounds(kWindowWidth / 2 + buttonWidth * 0.5, y, buttonWidth, kButtonBaseHeight);
  fToJavaButton->onClick = [this]() {
    fTo = To::Java;
    if (fFrom == From::Java) {
      fFrom = std::nullopt;
    } else if (fLastFrom != From::Java) {
      fFrom = fLastFrom;
    }
    fLastTo = To::Java;
    update();
  };
  fToJavaButton->setToggleable(true);
  addAndMakeVisible(*fToJavaButton);
  y += kButtonBaseHeight;

  y += kMargin;

  fFromXbox360Button.reset(new TextButton(TRANS("from Xbox360")));
  fFromXbox360Button->setBounds(kWindowWidth / 2 - buttonWidth * 1.5, y, buttonWidth, kButtonBaseHeight);
  fFromXbox360Button->onClick = [this]() {
    fFrom = From::Xbox360;
    if (!fTo) {
      fTo = fLastTo;
    }
    fLastFrom = From::Xbox360;
    update();
  };
  fFromXbox360Button->setToggleable(true);
  addAndMakeVisible(*fFromXbox360Button);

  fToBedrockButton.reset(new TextButton(TRANS("to Bedrock")));
  fToBedrockButton->setBounds(kWindowWidth / 2 + buttonWidth * 0.5, y, buttonWidth, kButtonBaseHeight);
  fToBedrockButton->onClick = [this]() {
    fTo = To::Bedrock;
    if (fFrom == From::Bedrock) {
      fFrom = std::nullopt;
    } else if (fLastFrom != From::Bedrock) {
      fFrom = fLastFrom;
    }
    fLastTo = To::Bedrock;
    update();
  };
  fToBedrockButton->setToggleable(true);
  addAndMakeVisible(*fToBedrockButton);
  y += kButtonBaseHeight;

  y += kMargin;

  fFromPS3Button.reset(new TextButton(TRANS("from PS3")));
  fFromPS3Button->setBounds(kWindowWidth / 2 - buttonWidth * 1.5, y, buttonWidth, kButtonBaseHeight);
  fFromPS3Button->onClick = [this]() {
    fFrom = From::PS3;
    if (!fTo) {
      fTo = fLastTo;
    }
    fLastFrom = From::PS3;
    update();
  };
  fFromPS3Button->setToggleable(true);
  addAndMakeVisible(*fFromPS3Button);
  y += fFromPS3Button->getHeight();

  fAboutButton.reset(new TextButton("About"));
  fAboutButton->setBounds(kMargin, kWindowHeight - kMargin - kButtonBaseHeight, kButtonMinWidth, kButtonBaseHeight);
  fAboutButton->onClick = [this]() { onAboutButtonClicked(); };
  addAndMakeVisible(*fAboutButton);

  fNextButton.reset(new component::TextButton(TRANS("Next")));
  fNextButton->setBounds(kWindowWidth - kMargin - kButtonMinWidth, kWindowHeight - kMargin - kButtonBaseHeight, kButtonMinWidth, kButtonBaseHeight);
  fNextButton->setEnabled(false);
  fNextButton->onClick = [this]() { onNextButtonClicked(); };
  addAndMakeVisible(*fNextButton);

  fJavaToBedrockPath = CreateButtonConnectingPath(*fFromJavaButton, *fToBedrockButton);
  fBedrockToJavaPath = CreateButtonConnectingPath(*fFromBedrockButton, *fToJavaButton);
  fXbox360ToJavaPath = CreateButtonConnectingPath(*fFromXbox360Button, *fToJavaButton);
  fXbox360ToBedrockPath = CreateButtonConnectingPath(*fFromXbox360Button, *fToBedrockButton);
  fPS3ToJavaPath = CreateButtonConnectingPath(*fFromPS3Button, *fToJavaButton);
  fPS3ToBedrockPath = CreateButtonConnectingPath(*fFromPS3Button, *fToBedrockButton);

  fArrowHeadJavaToBedrock = CreateArrowHead(*fFromJavaButton, *fToBedrockButton);
  fArrowHeadBedrockToJava = CreateArrowHead(*fFromBedrockButton, *fToJavaButton);
  fArrowHeadXbox360ToJava = CreateArrowHead(*fFromXbox360Button, *fToJavaButton);
  fArrowHeadXbox360ToBedrock = CreateArrowHead(*fFromXbox360Button, *fToBedrockButton);
  fArrowHeadPS3ToJava = CreateArrowHead(*fFromPS3Button, *fToJavaButton);
  fArrowHeadPS3ToBedrock = CreateArrowHead(*fFromPS3Button, *fToBedrockButton);
}

ModeSelect::~ModeSelect() {
}

void ModeSelect::paint(juce::Graphics &g) {
  g.setColour(findColour(juce::TextButton::ColourIds::textColourOnId));
  juce::PathStrokeType type(10, juce::PathStrokeType::JointStyle::curved, juce::PathStrokeType::EndCapStyle::rounded);
  if (fFrom != From::Java || fTo != To::Bedrock) {
    g.strokePath(fJavaToBedrockPath, type);
  }
  if (fFrom != From::Bedrock || fTo != To::Java) {
    g.strokePath(fBedrockToJavaPath, type);
  }
  if (fFrom != From::Xbox360 || fTo != To::Java) {
    g.strokePath(fXbox360ToJavaPath, type);
  }
  if (fFrom != From::Xbox360 || fTo != To::Bedrock) {
    g.strokePath(fXbox360ToBedrockPath, type);
  }
  if (fFrom != From::PS3 || fTo != To::Java) {
    g.strokePath(fPS3ToJavaPath, type);
  }
  if (fFrom != From::PS3 || fTo != To::Bedrock) {
    g.strokePath(fPS3ToBedrockPath, type);
  }
  g.setColour(findColour(juce::TextButton::ColourIds::textColourOffId));
  if (fFrom == From::Java && fTo == To::Bedrock) {
    g.strokePath(fJavaToBedrockPath, type);
    g.strokePath(fArrowHeadJavaToBedrock, type);
  }
  if (fFrom == From::Bedrock && fTo == To::Java) {
    g.strokePath(fBedrockToJavaPath, type);
    g.strokePath(fArrowHeadBedrockToJava, type);
  }
  if (fFrom == From::Xbox360 && fTo == To::Java) {
    g.strokePath(fXbox360ToJavaPath, type);
    g.strokePath(fArrowHeadXbox360ToJava, type);
  }
  if (fFrom == From::Xbox360 && fTo == To::Bedrock) {
    g.strokePath(fXbox360ToBedrockPath, type);
    g.strokePath(fArrowHeadXbox360ToBedrock, type);
  }
  if (fFrom == From::PS3 && fTo == To::Java) {
    g.strokePath(fPS3ToJavaPath, type);
    g.strokePath(fArrowHeadPS3ToJava, type);
  }
  if (fFrom == From::PS3 && fTo == To::Bedrock) {
    g.strokePath(fPS3ToBedrockPath, type);
    g.strokePath(fArrowHeadPS3ToBedrock, type);
  }
}

void ModeSelect::update() {
  fFromJavaButton->setToggleState(fFrom == From::Java, juce::dontSendNotification);
  fFromBedrockButton->setToggleState(fFrom == From::Bedrock, juce::dontSendNotification);
  fFromXbox360Button->setToggleState(fFrom == From::Xbox360, juce::dontSendNotification);
  fFromPS3Button->setToggleState(fFrom == From::PS3, juce::dontSendNotification);
  fToJavaButton->setToggleState(fTo == To::Java, juce::dontSendNotification);
  fToBedrockButton->setToggleState(fTo == To::Bedrock, juce::dontSendNotification);
  fNextButton->setEnabled(fFrom && fTo);

  repaint();
}

void ModeSelect::onAboutButtonClicked() {
  DialogWindow::LaunchOptions options;
  options.content.setOwned(new About());
  options.dialogTitle = "About";
  options.useNativeTitleBar = true;
  options.escapeKeyTriggersCloseButton = true;
  options.resizable = false;
  options.dialogBackgroundColour = getLookAndFeel().findColour(ResizableWindow::backgroundColourId);
  options.launchAsync();
}

void ModeSelect::onNextButtonClicked() {
  if (!fFrom || !fTo) {
    return;
  }
  switch (*fFrom) {
  case From::Java:
    if (fTo == To::Bedrock) {
      JUCEApplication::getInstance()->invoke(commands::toChooseJavaInput, true);
    }
    break;
  case From::Bedrock:
    if (fTo == To::Java) {
      JUCEApplication::getInstance()->invoke(commands::toChooseBedrockInput, true);
    }
    break;
  case From::Xbox360:
    if (fTo == To::Java) {
      JUCEApplication::getInstance()->invoke(commands::toChooseXbox360InputToJava, true);
    } else {
      JUCEApplication::getInstance()->invoke(commands::toChooseXbox360InputToBedrock, true);
    }
    break;
  case From::PS3:
    if (fTo == To::Java) {
      JUCEApplication::getInstance()->invoke(commands::toChoosePS3InputToJava, true);
    } else {
      JUCEApplication::getInstance()->invoke(commands::toChoosePS3InputToBedrock, true);
    }
    break;
  }
}

} // namespace je2be::desktop::component
