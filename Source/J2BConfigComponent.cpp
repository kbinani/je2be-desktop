#include "J2BConfigComponent.h"
#include "CommandID.h"
#include "Constants.h"

using namespace juce;

namespace je2be::gui {

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

ConfigComponent::ConfigComponent(J2BChooseInputState const &chooseInputState) : fState(chooseInputState) {
  auto width = kWindowWidth;
  auto height = kWindowHeight;
  setSize(width, height);

  String label = (*fState.fInputState.fInputDirectory).getFullPathName();
  fDirectory.reset(new Label("", TRANS("Selected world:") + " " + label));
  fDirectory->setBounds(kMargin, kMargin, width - kMargin * 2, kButtonBaseHeight);
  fDirectory->setJustificationType(Justification::topLeft);
  addAndMakeVisible(*fDirectory);

  fStartButton.reset(new TextButton(TRANS("Start")));
  fStartButton->setBounds(width - kMargin - kButtonMinWidth, height - kMargin - kButtonBaseHeight, kButtonMinWidth, kButtonBaseHeight);
  fStartButton->setEnabled(false);
  fStartButton->onClick = [this]() { onStartButtonClicked(); };
  addAndMakeVisible(*fStartButton);

  fBackButton.reset(new TextButton(TRANS("Back")));
  fBackButton->setBounds(kMargin, height - kMargin - kButtonBaseHeight, kButtonMinWidth, kButtonBaseHeight);
  fBackButton->setMouseCursor(MouseCursor::PointingHandCursor);
  fBackButton->onClick = [this]() { onBackButtonClicked(); };
  addAndMakeVisible(*fBackButton);

  if (fState.fInputState.fInputDirectory) {
    auto structure = SniffDirectoryStructure(*fState.fInputState.fInputDirectory);
    if (structure) {
      fState.fStructure = *structure;
      fOk = true;
    }
  }

  if (fOk) {
    String s = "Vanilla";
    if (fState.fStructure == J2BConfigState::DirectoryStructure::Paper) {
      s = "Spigot/Paper";
    }
    fMessage.reset(new Label("", TRANS("Directory structure") + ": " + s));
  } else {
    fMessage.reset(new Label("", TRANS("There doesn't seem to be any Minecraft save data "
                                       "in the specified directory.")));
    fMessage->setColour(Label::textColourId, kErrorTextColor);
  }
  fMessage->setBounds(kMargin, kMargin + kButtonBaseHeight + kMargin, width - 2 * kMargin, kButtonBaseHeight);
  addAndMakeVisible(*fMessage);

  startTimer(1000);
}

ConfigComponent::~ConfigComponent() {}

void ConfigComponent::timerCallback() {
  stopTimer();
  fStartButton->setEnabled(fOk);
  if (fOk) {
    fStartButton->setMouseCursor(MouseCursor::PointingHandCursor);
  }
}

void ConfigComponent::paint(juce::Graphics &g) {}

void ConfigComponent::onStartButtonClicked() {
  JUCEApplication::getInstance()->invoke(gui::toConvert, true);
}

void ConfigComponent::onBackButtonClicked() {
  JUCEApplication::getInstance()->invoke(gui::toChooseInput, true);
}

} // namespace je2be::gui
