#include "component/ModeSelect.h"
#include "CommandID.h"
#include "Constants.h"
#include "component/About.h"
#include "component/TextButton.h"

using namespace juce;

namespace je2be::gui::component {

ModeSelect::ModeSelect() {
  setSize(kWindowWidth, kWindowHeight);

  int const buttonWidth = kWindowWidth / 2;

  int y = kMargin;
  fLabel.reset(new Label("", TRANS("Select conversion mode") + ":"));
  fLabel->setJustificationType(Justification::centred);
  fLabel->setBounds(kMargin, y, kWindowWidth - 2 * kMargin, kButtonBaseHeight);
  addAndMakeVisible(*fLabel);
  y += fLabel->getHeight();

  y += kMargin;

  fToJ2B.reset(new TextButton(TRANS("Convert Java map")));
  fToJ2B->setBounds(kWindowWidth / 2 - buttonWidth / 2, y, buttonWidth, kButtonBaseHeight);
  fToJ2B->onClick = [this]() { onJ2BClicked(); };
  addAndMakeVisible(*fToJ2B);
  y += fToJ2B->getHeight();

  y += kMargin;

  fToB2J.reset(new TextButton(TRANS("Convert Bedrock map")));
  fToB2J->setBounds(kWindowWidth / 2 - buttonWidth / 2, y, buttonWidth, kButtonBaseHeight);
  fToB2J->onClick = [this]() { onB2JClicked(); };
  addAndMakeVisible(*fToB2J);
  y += fToB2J->getHeight();

  fAboutButton.reset(new TextButton("About"));
  fAboutButton->setBounds(kMargin, kWindowHeight - kMargin - kButtonBaseHeight, kButtonMinWidth, kButtonBaseHeight);
  fAboutButton->onClick = [this]() { onAboutButtonClicked(); };
  addAndMakeVisible(*fAboutButton);
}

ModeSelect::~ModeSelect() {
}

void ModeSelect::paint(juce::Graphics &g) {
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

void ModeSelect::onB2JClicked() {
  JUCEApplication::getInstance()->invoke(gui::toChooseBedrockInput, true);
}

void ModeSelect::onJ2BClicked() {
  JUCEApplication::getInstance()->invoke(gui::toChooseJavaInput, true);
}

} // namespace je2be::gui::component
