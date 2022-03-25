#include "component/SelectInputType.h"
#include "CommandID.h"
#include "Constants.h"
#include "component/About.h"
#include "component/TextButton.h"

using namespace juce;

namespace je2be::gui::component {

SelectInputType::SelectInputType() {
  setSize(kWindowWidth, kWindowHeight);

  int const buttonWidth = kWindowWidth / 2;

  int y = kMargin;
  fLabel.reset(new Label("", TRANS("Select the game edition of map to convert") + ":"));
  fLabel->setJustificationType(Justification::centred);
  fLabel->setBounds(kMargin, y, kWindowWidth - 2 * kMargin, kButtonBaseHeight);
  addAndMakeVisible(*fLabel);
  y += fLabel->getHeight();

  y += kMargin;

  fConvertJava.reset(new TextButton(TRANS("Convert Java map")));
  fConvertJava->setBounds(kWindowWidth / 2 - buttonWidth / 2, y, buttonWidth, kButtonBaseHeight);
  fConvertJava->onClick = [this]() { onConvertJavaClicked(); };
  addAndMakeVisible(*fConvertJava);
  y += fConvertJava->getHeight();

  y += kMargin;

  fConvertBedrock.reset(new TextButton(TRANS("Convert Bedrock map")));
  fConvertBedrock->setBounds(kWindowWidth / 2 - buttonWidth / 2, y, buttonWidth, kButtonBaseHeight);
  fConvertBedrock->onClick = [this]() { onConvertBedrockClicked(); };
  addAndMakeVisible(*fConvertBedrock);
  y += fConvertBedrock->getHeight();

  fAboutButton.reset(new TextButton("About"));
  fAboutButton->setBounds(kMargin, kWindowHeight - kMargin - kButtonBaseHeight, kButtonMinWidth, kButtonBaseHeight);
  fAboutButton->onClick = [this]() { onAboutButtonClicked(); };
  addAndMakeVisible(*fAboutButton);
}

SelectInputType::~SelectInputType() {
}

void SelectInputType::paint(juce::Graphics &g) {
}

void SelectInputType::onAboutButtonClicked() {
  DialogWindow::LaunchOptions options;
  options.content.setOwned(new About());
  options.dialogTitle = "About";
  options.useNativeTitleBar = true;
  options.escapeKeyTriggersCloseButton = true;
  options.resizable = false;
  options.dialogBackgroundColour = getLookAndFeel().findColour(ResizableWindow::backgroundColourId);
  options.launchAsync();
}

void SelectInputType::onConvertBedrockClicked() {
  JUCEApplication::getInstance()->invoke(gui::toChooseBedrockInput, true);
}

void SelectInputType::onConvertJavaClicked() {
  JUCEApplication::getInstance()->invoke(gui::toChooseJavaInput, true);
}

} // namespace je2be::gui::component
