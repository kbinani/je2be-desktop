#include "component/j2b/J2BConfig.h"
#include "CommandID.h"
#include "Constants.h"
#include "component/TextButton.h"

using namespace juce;

namespace je2be::desktop::component::j2b {

static std::optional<J2BConfigState::DirectoryStructure> SniffDirectoryStructure(File input) {
  File vanillaLevelDat = input.getChildFile("level.dat");
  if (vanillaLevelDat.existsAsFile()) {
    return J2BConfigState::DirectoryStructure::Vanilla;
  }
  File paperLevelDat = input.getChildFile("world").getChildFile("level.dat");
  if (paperLevelDat.existsAsFile()) {
    return J2BConfigState::DirectoryStructure::Paper;
  }
  return std::nullopt;
}

J2BConfig::J2BConfig(ChooseInputState const &chooseInputState) : fState(chooseInputState) {
  auto width = kWindowWidth;
  auto height = kWindowHeight;
  setSize(width, height);

  String label = fState.fInputState.fInput.getFullPathName();
  fDirectory.reset(new Label("", TRANS("Selected world:") + " " + label));
  fDirectory->setBounds(kMargin, kMargin, width - kMargin * 2, kButtonBaseHeight);
  fDirectory->setJustificationType(Justification::topLeft);
  addAndMakeVisible(*fDirectory);

  fStartButton.reset(new component::TextButton(TRANS("Start")));
  fStartButton->setBounds(width - kMargin - kButtonMinWidth, height - kMargin - kButtonBaseHeight, kButtonMinWidth, kButtonBaseHeight);
  fStartButton->setEnabled(false);
  fStartButton->onClick = [this]() { onStartButtonClicked(); };
  addAndMakeVisible(*fStartButton);

  fBackButton.reset(new component::TextButton(TRANS("Back")));
  fBackButton->setBounds(kMargin, height - kMargin - kButtonBaseHeight, kButtonMinWidth, kButtonBaseHeight);
  fBackButton->onClick = [this]() { onBackButtonClicked(); };
  addAndMakeVisible(*fBackButton);

  auto structure = SniffDirectoryStructure(fState.fInputState.fInput);
  if (structure) {
    fState.fStructure = *structure;
    fOk = true;
  }

  if (fOk) {
    String s = "Vanilla";
    if (fState.fStructure == J2BConfigState::DirectoryStructure::Paper) {
      s = "Spigot/Paper";
    }
    fMessage.reset(new Label("", TRANS("Directory structure") + ": " + s));
  } else {
    fMessage.reset(new Label("", TRANS("There doesn't seem to be any Minecraft save data in the specified folder.")));
    fMessage->setColour(Label::textColourId, kErrorTextColor);
  }
  fMessage->setBounds(kMargin, kMargin + kButtonBaseHeight + kMargin, width - 2 * kMargin, kButtonBaseHeight);
  addAndMakeVisible(*fMessage);

  startTimer(1000);
}

J2BConfig::~J2BConfig() {}

void J2BConfig::timerCallback() {
  stopTimer();
  fStartButton->setEnabled(fOk);
}

void J2BConfig::paint(juce::Graphics &g) {}

void J2BConfig::onStartButtonClicked() {
  JUCEApplication::getInstance()->invoke(commands::toJ2BConvert, true);
}

void J2BConfig::onBackButtonClicked() {
  JUCEApplication::getInstance()->invoke(commands::toChooseJavaInput, true);
}

} // namespace je2be::desktop::component::j2b
