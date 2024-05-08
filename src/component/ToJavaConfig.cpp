#include <je2be.hpp>

#include "CommandID.h"
#include "Constants.h"
#include "GameDirectory.h"
#include "component/TextButton.h"
#include "component/ToJavaConfig.h"

using namespace juce;

namespace je2be::desktop::component {

ToJavaConfig::ToJavaConfig(ChooseInputState const &chooseInputState, int forwardCommand, int backwardCommand) : fState(chooseInputState), fForwardCommand(forwardCommand), fBackwardCommand(backwardCommand) {
  auto width = kWindowWidth;
  auto height = kWindowHeight;
  setSize(width, height);

  int y = kMargin;
  juce::String label = fState.fInputState.fInput.getFullPathName();
  fFileOrDirectory.reset(new Label("", TRANS("Selected world:") + " " + label));
  fFileOrDirectory->setBounds(kMargin, kMargin, width - kMargin * 2, kButtonBaseHeight);
  fFileOrDirectory->setJustificationType(Justification::topLeft);
  addAndMakeVisible(*fFileOrDirectory);

  {
    y += 3 * kMargin;
    fImportAccountFromLauncher.reset(new ToggleButton(TRANS("Import player ID from the Minecraft Launcher to embed it into level.dat as a local player ID")));
    fImportAccountFromLauncher->setBounds(kMargin, y, width - kMargin * 2, kButtonBaseHeight);
    fImportAccountFromLauncher->setMouseCursor(MouseCursor::PointingHandCursor);
    fImportAccountFromLauncher->onClick = [this] {
      onClickImportAccountFromLauncherButton();
    };
    addAndMakeVisible(*fImportAccountFromLauncher);
    y += fImportAccountFromLauncher->getHeight();
  }

  {
    int x = 42;

    juce::String title = TRANS("Account:") + " ";
    fAccountListLabel.reset(new Label("", title));
    int labelWidth = getLookAndFeel().getLabelFont(*fAccountListLabel).getStringWidth(title) + 10;
    fAccountListLabel->setBounds(x, y, labelWidth, kButtonBaseHeight);
    fAccountListLabel->setJustificationType(Justification::centredLeft);
    addAndMakeVisible(*fAccountListLabel);

    fAccountList.reset(new ComboBox());
    fAccountList->setBounds(x + labelWidth, y, width - x - labelWidth - kMargin, kButtonBaseHeight);
    fAccountList->setEnabled(false);
    addAndMakeVisible(*fAccountList);
    y += fAccountList->getHeight();
  }

  int messageComponentY = y + kMargin;

  fStartButton.reset(new component::TextButton(TRANS("Start")));
  fStartButton->setBounds(width - kMargin - kButtonMinWidth, height - kMargin - kButtonBaseHeight, kButtonMinWidth, kButtonBaseHeight);
  fStartButton->setEnabled(false);
  fStartButton->onClick = [this]() { onStartButtonClicked(); };
  addAndMakeVisible(*fStartButton);

  fBackButton.reset(new component::TextButton(TRANS("Back")));
  fBackButton->setBounds(kMargin, height - kMargin - kButtonBaseHeight, kButtonMinWidth, kButtonBaseHeight);
  fBackButton->onClick = [this]() { onBackButtonClicked(); };
  addAndMakeVisible(*fBackButton);

  juce::String error;
  juce::File input = fState.fInputState.fInput;
  switch (fState.fInputState.fType) {
  case InputType::Bedrock: {
    if (input.isDirectory()) {
      if (!input.getChildFile("level.dat").existsAsFile()) {
        error = TRANS("level.dat file not found");
        break;
      } else if (!input.getChildFile("db").isDirectory()) {
        error = TRANS("db directory not found");
        break;
      } else if (!input.getChildFile("db").getChildFile("CURRENT").existsAsFile()) {
        error = TRANS("CURRENT file not found");
        break;
      }
    } else {
      auto extension = input.getFileExtension().toLowerCase();
      if (extension != ".mcworld" && extension != ".zip") {
        error = TRANS("Unsupported file type") + ": \"" + input.getFileExtension() + "\"";
      }
    }
    break;
  }
  case InputType::Xbox360: {
    auto extension = input.getFileExtension().toLowerCase();
    if (extension != ".bin") {
      error = TRANS("Unsupported file type") + ": \"" + input.getFileExtension() + "\"";
    }
    break;
  }
  case InputType::PS3:
    break;
  case InputType::Java:
  default:
    error = "Unexpected input type: InputType::Java";
    break;
  }

  if (error.isEmpty()) {
    fMessage.reset(new Label("", ""));
  } else {
    fMessage.reset(new Label("", TRANS("There doesn't seem to be any Minecraft save data in the specified folder.")));
    fMessage->setColour(Label::textColourId, kErrorTextColor);
  }
  fMessage->setBounds(kMargin, messageComponentY, width - 2 * kMargin, kButtonBaseHeight);
  addAndMakeVisible(*fMessage);

  fImportAccountFromLauncher->setEnabled(false);
  fStartButton->setEnabled(false);
  fBackButton->setEnabled(false);
  fAccountScanThread.reset(new AccountScanThread(this));
  fAccountScanThread->startThread();

  startTimer(1000);
}

ToJavaConfig::~ToJavaConfig() {}

void ToJavaConfig::timerCallback() {
  stopTimer();
  updateStartButton();
}

void ToJavaConfig::updateStartButton() {
  if (fAccountScanThread == nullptr && !isTimerRunning()) {
    fStartButton->setEnabled(fOk);
  } else {
    fStartButton->setEnabled(false);
  }
}

void ToJavaConfig::paint(juce::Graphics &g) {}

void ToJavaConfig::onStartButtonClicked() {
  if (fImportAccountFromLauncher->getToggleState()) {
    int selected = fAccountList->getSelectedId();
    int index = selected - 1;
    if (0 <= index && index < fAccounts.size()) {
      Account a = fAccounts[index];
      fState.fLocalPlayer = a.fUuid;
    }
  }
  JUCEApplication::getInstance()->invoke(fForwardCommand, true);
}

void ToJavaConfig::onBackButtonClicked() {
  JUCEApplication::getInstance()->invoke(fBackwardCommand, true);
}

void ToJavaConfig::onClickImportAccountFromLauncherButton() {
  if (fAccountScanThread) {
    return;
  }
  fAccountList->setEnabled(fImportAccountFromLauncher->getToggleState());
}

void ToJavaConfig::handleAsyncUpdate() {
  if (fAccountScanThread) {
    fAccountScanThread->copyAccounts(fAccounts);
  }
  fAccountScanThread.reset();
  fAccountList->clear();
  for (int i = 0; i < fAccounts.size(); i++) {
    Account account = fAccounts[i];
    fAccountList->addItem(account.toString(), i + 1);
  }
  if (fAccounts.size() > 0) {
    fAccountList->setSelectedId(1);
  }
  fAccountList->setEnabled(true);
  fImportAccountFromLauncher->setEnabled(true);
  fImportAccountFromLauncher->setToggleState(fAccounts.size() > 0, dontSendNotification);
  updateStartButton();
  fBackButton->setEnabled(true);
}

} // namespace je2be::desktop::component
