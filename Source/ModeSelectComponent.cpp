#include "ModeSelectComponent.h"
#include "AboutComponent.h"
#include "CommandID.h"
#include "Constants.h"

using namespace juce;

namespace je2be::gui {

ModeSelectComponent::ModeSelectComponent() {
  setSize(kWindowWidth, kWindowHeight);

  int const buttonWidth = kWindowWidth / 2;

  int y = kMargin;
  fLabel.reset(new Label("", TRANS("Select conversion mode:")));
  fLabel->setJustificationType(Justification::centred);
  fLabel->setBounds(kMargin, y, kWindowWidth - 2 * kMargin, kButtonBaseHeight);
  addAndMakeVisible(*fLabel);
  y += fLabel->getHeight();

  y += kMargin;

  fToJ2B.reset(new TextButton(TRANS("Java to Bedrock")));
  fToJ2B->setBounds(kWindowWidth / 2 - buttonWidth / 2, y, buttonWidth, kButtonBaseHeight);
  fToJ2B->setMouseCursor(MouseCursor::PointingHandCursor);
  fToJ2B->onClick = [this]() { onJ2BClicked(); };
  addAndMakeVisible(*fToJ2B);
  y += fToJ2B->getHeight();

  y += kMargin;

  fToB2J.reset(new TextButton(TRANS("Bedrock to Java")));
  fToB2J->setBounds(kWindowWidth / 2 - buttonWidth / 2, y, buttonWidth, kButtonBaseHeight);
  fToB2J->setMouseCursor(MouseCursor::PointingHandCursor);
  fToB2J->onClick = [this]() { onB2JClicked(); };
  addAndMakeVisible(*fToB2J);
  y += fToB2J->getHeight();

  fAboutButton.reset(new TextButton("About"));
  fAboutButton->setBounds(kMargin, kWindowHeight - kMargin - kButtonBaseHeight, kButtonMinWidth, kButtonBaseHeight);
  fAboutButton->setMouseCursor(MouseCursor::PointingHandCursor);
  fAboutButton->onClick = [this]() { onAboutButtonClicked(); };
  addAndMakeVisible(*fAboutButton);
}

ModeSelectComponent::~ModeSelectComponent() {
}

void ModeSelectComponent::paint(juce::Graphics &g) {
}

void ModeSelectComponent::onAboutButtonClicked() {
  DialogWindow::LaunchOptions options;
  options.content.setOwned(new AboutComponent());
  options.dialogTitle = "About";
  options.useNativeTitleBar = true;
  options.escapeKeyTriggersCloseButton = true;
  options.resizable = false;
  options.dialogBackgroundColour = getLookAndFeel().findColour(ResizableWindow::backgroundColourId);
  options.launchAsync();
}

void ModeSelectComponent::onB2JClicked() {
  //JUCEApplication::getInstance()->invoke(gui::toB2JConfig, false);
}

void ModeSelectComponent::onJ2BClicked() {
  JUCEApplication::getInstance()->invoke(gui::toJ2BChooseInput, true);
}

} // namespace je2be::gui
