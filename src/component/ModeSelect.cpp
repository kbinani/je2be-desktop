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

  fJavaToBedrockButton.reset(new TextButton(TRANS("Java to Bedrock")));
  fJavaToBedrockButton->setBounds(kWindowWidth / 2 - buttonWidth / 2, y, buttonWidth, kButtonBaseHeight);
  fJavaToBedrockButton->onClick = [this]() { onJavaToBedrockButtonClicked(); };
  addAndMakeVisible(*fJavaToBedrockButton);
  y += fJavaToBedrockButton->getHeight();

  y += kMargin;

  fBedrockToJavaButton.reset(new TextButton(TRANS("Bedrock to Java")));
  fBedrockToJavaButton->setBounds(kWindowWidth / 2 - buttonWidth / 2, y, buttonWidth, kButtonBaseHeight);
  fBedrockToJavaButton->onClick = [this]() { onBedrockToJavaButtonClicked(); };
  addAndMakeVisible(*fBedrockToJavaButton);
  y += fBedrockToJavaButton->getHeight();

  y += kMargin;

  fXbox360ToBedrockButton.reset(new TextButton(TRANS("Xbox360 to Bedrock")));
  fXbox360ToBedrockButton->setBounds(kWindowWidth / 2 - buttonWidth / 2, y, buttonWidth, kButtonBaseHeight);
  fXbox360ToBedrockButton->onClick = [this]() { onXbox360ToBedrockButtonClicked(); };
  addAndMakeVisible(*fXbox360ToBedrockButton);
  y += fXbox360ToBedrockButton->getHeight();

  y += kMargin;

  fXbox360ToJavaButton.reset(new TextButton(TRANS("Xbox360 to Java")));
  fXbox360ToJavaButton->setBounds(kWindowWidth / 2 - buttonWidth / 2, y, buttonWidth, kButtonBaseHeight);
  fXbox360ToJavaButton->onClick = [this]() { onXbox360ToJavaButtonClicked(); };
  addAndMakeVisible(*fXbox360ToJavaButton);
  y += fXbox360ToJavaButton->getHeight();

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

void ModeSelect::onBedrockToJavaButtonClicked() {
  JUCEApplication::getInstance()->invoke(gui::toChooseBedrockInput, true);
}

void ModeSelect::onJavaToBedrockButtonClicked() {
  JUCEApplication::getInstance()->invoke(gui::toChooseJavaInput, true);
}

void ModeSelect::onXbox360ToJavaButtonClicked() {
  JUCEApplication::getInstance()->invoke(gui::toChooseXbox360InputToJava, true);
}

void ModeSelect::onXbox360ToBedrockButtonClicked() {
  JUCEApplication::getInstance()->invoke(gui::toChooseXbox360InputToBedrock, true);
}

} // namespace je2be::gui::component
