#include <je2be.hpp>

#include "CommandID.h"
#include "Constants.h"
#include "File.h"
#include "GameDirectoryScanThreadXbox360.h"
#include "component/ChooseXbox360Input.h"
#include "component/MainWindow.h"
#include "component/TextButton.h"

using namespace juce;

namespace je2be::desktop::component {

File ChooseXbox360Input::sLastDirectory;

static juce::String GetWorldName(File input, std::vector<GameDirectory> const &buffer) {
  for (auto const &gd : buffer) {
    if (gd.fDirectory == input) {
      return gd.fLevelName;
    }
  }

  auto parent = input.getParentDirectory();
  auto saveInfo = parent.getChildFile("_MinecraftSaveInfo");
  if (!saveInfo.existsAsFile()) {
    return input.getFileNameWithoutExtension();
  }
  std::vector<je2be::box360::MinecraftSaveInfo::SaveBin> bins;
  je2be::box360::MinecraftSaveInfo::Parse(PathFromFile(saveInfo), bins);
  for (auto const &bin : bins) {
    if (input.getFileName() == juce::String(bin.fFileName)) {
      std::u16string const &title = bin.fTitle;
      juce::String name(CharPointer_UTF16((CharPointer_UTF16::CharType const *)title.c_str()), title.size());
      return name;
    }
  }

  return input.getFileNameWithoutExtension();
}

ChooseXbox360Input::ChooseXbox360Input(juce::CommandID destinationAfterChoose, std::optional<ChooseInputState> state) : fDestinationAfterChoose(destinationAfterChoose) {
  if (state) {
    fState = *state;
  }

  auto width = kWindowWidth;
  auto height = kWindowHeight;

  fBedrockGameDirectory = GameDirectory::BedrockSaveDirectory();

  setSize(width, height);
  {
    int y = kMargin;
    fMessage.reset(new Label("", TRANS("Select a world to convert\nfrom the list")));
    auto borderSize = getLookAndFeel().getLabelBorderSize(*fMessage);
    auto lineHeight = getLookAndFeel().getLabelFont(*fMessage).getHeight();
    fMessage->setBounds(kMargin, y, width - kMargin - kWorldListWidth - kMargin - kMargin, lineHeight * 2 + borderSize.getTopAndBottom());
    fMessage->setJustificationType(Justification::topLeft);
    fMessage->setMinimumHorizontalScale(1);
    addAndMakeVisible(*fMessage);
    y += fMessage->getHeight();

    y += kMargin;
    fOrMessage.reset(new Label("", TRANS("or")));
    fOrMessage->setBounds(kMargin + kMargin, y, fMessage->getWidth() - kMargin, lineHeight + borderSize.getTopAndBottom());
    fOrMessage->setJustificationType(Justification::topLeft);
    addAndMakeVisible(*fOrMessage);
    y += fOrMessage->getHeight();

    y += kMargin;
    fChooseCustomButton.reset(new TextButton(TRANS("Select *.bin file")));
    fChooseCustomButton->setBounds(kMargin, y, fMessage->getWidth(), kButtonBaseHeight);
    fChooseCustomButton->changeWidthToFitText();
    fChooseCustomButton->setSize(jmin(fMessage->getWidth(), fChooseCustomButton->getWidth() + 2 * kMargin), fChooseCustomButton->getHeight());
    fChooseCustomButton->onClick = [this]() { onChooseCustomButtonClicked(); };
    addAndMakeVisible(*fChooseCustomButton);
    y += fChooseCustomButton->getHeight();
  }
  {
    fBackButton.reset(new TextButton(TRANS("Back")));
    fBackButton->setBounds(kMargin, height - kMargin - kButtonBaseHeight, kButtonMinWidth, kButtonBaseHeight);
    fBackButton->onClick = [this]() { onBackButtonClicked(); };
    addAndMakeVisible(*fBackButton);
  }
  {
    fNextButton.reset(new TextButton(TRANS("Next")));
    fNextButton->setBounds(width - kButtonMinWidth - kMargin, height - kButtonBaseHeight - kMargin, kButtonMinWidth, kButtonBaseHeight);
    fNextButton->onClick = [this]() { onNextButtonClicked(); };
    fNextButton->setEnabled(false);
    addAndMakeVisible(*fNextButton);
  }

  juce::Rectangle<int> listBoxBounds(width - kMargin - kWorldListWidth, kMargin, kWorldListWidth, height - 3 * kMargin - kButtonBaseHeight);
  {
    fListComponent.reset(new ListBox("", this));
    fListComponent->setBounds(listBoxBounds);
    fListComponent->setEnabled(false);
    fListComponent->setRowHeight(kWorldListRowHeight);
    addChildComponent(*fListComponent);
  }
  {
    fPlaceholder.reset(new Label({}, TRANS("Loading worlds in the save folder...")));
    fPlaceholder->setBounds(listBoxBounds);
    fPlaceholder->setColour(Label::ColourIds::backgroundColourId, fListComponent->findColour(ListBox::ColourIds::backgroundColourId));
    fPlaceholder->setJustificationType(Justification::centred);
    addAndMakeVisible(*fPlaceholder);
  }
  {
    fThread.reset(new GameDirectoryScanThreadXbox360(this));
    fThread->startThread();
  }
}

ChooseXbox360Input::~ChooseXbox360Input() {
  fListComponent.reset();
  fThread->stopThread(-1);
}

void ChooseXbox360Input::paint(juce::Graphics &g) {}

void ChooseXbox360Input::onNextButtonClicked() {
  JUCEApplication::getInstance()->invoke(fDestinationAfterChoose, true);
}

void ChooseXbox360Input::onChooseCustomButtonClicked() {
  int flags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles;
  MainWindow::sFileChooser.reset(new FileChooser(TRANS("Select *.bin file to convert"), sLastDirectory, "*.bin", true));
  MainWindow::sFileChooser->launchAsync(flags, [this](FileChooser const &chooser) { onCustomDirectorySelected(chooser); });
}

void ChooseXbox360Input::onCustomDirectorySelected(juce::FileChooser const &chooser) {
  File result = chooser.getResult();
  if (result == File()) {
    return;
  }
  juce::String worldName = GetWorldName(result, fGameDirectories);
  fState = ChooseInputState(InputType::Xbox360, result, worldName);
  sLastDirectory = result.getParentDirectory();
  JUCEApplication::getInstance()->invoke(fDestinationAfterChoose, true);
}

void ChooseXbox360Input::selectedRowsChanged(int lastRowSelected) {
  int num = fListComponent->getNumSelectedRows();
  if (num == 1 && 0 <= lastRowSelected && lastRowSelected < fGameDirectories.size()) {
    GameDirectory gd = fGameDirectories[lastRowSelected];
    juce::String worldName = gd.fLevelName;
    fState = ChooseInputState(InputType::Xbox360, gd.fDirectory, worldName);
  } else {
    fState = std::nullopt;
  }
  if (fState != std::nullopt) {
    fNextButton->setEnabled(true);
  }
}

void ChooseXbox360Input::listBoxItemDoubleClicked(int row, const MouseEvent &) {
  if (row < 0 || fGameDirectories.size() <= row) {
    return;
  }
  GameDirectory gd = fGameDirectories[row];
  juce::String worldName = gd.fLevelName;
  fState = ChooseInputState(InputType::Xbox360, gd.fDirectory, worldName);
  JUCEApplication::getInstance()->invoke(fDestinationAfterChoose, false);
}

void ChooseXbox360Input::onBackButtonClicked() {
  JUCEApplication::getInstance()->invoke(commands::toModeSelect, true);
}

int ChooseXbox360Input::getNumRows() {
  return fGameDirectories.size();
}

void ChooseXbox360Input::paintListBoxItem(int rowNumber,
                                          juce::Graphics &g,
                                          int width, int height,
                                          bool rowIsSelected) {
  if (rowNumber < 0 || fGameDirectories.size() <= rowNumber) {
    return;
  }
  GameDirectory gd = fGameDirectories[rowNumber];
  gd.paint(g, width, height, rowIsSelected, *this);
}

void ChooseXbox360Input::handleAsyncUpdate() {
  fGameDirectories.swap(fThread->fGameDirectories);
  if (fGameDirectories.empty()) {
    fPlaceholder->setText(TRANS("Nothing found in the save folder"), dontSendNotification);
  } else {
    fListComponent->updateContent();
    fListComponent->setEnabled(true);
    fListComponent->setVisible(true);
    fPlaceholder->setVisible(false);
  }
}

} // namespace je2be::desktop::component
