#include "component/ChooseBedrockInput.h"
#include "CommandID.h"
#include "Constants.h"
#include "GameDirectoryScanThreadBedrock.h"
#include "component/MainWindow.h"
#include "component/TextButton.h"

using namespace juce;

namespace je2be::gui::component {

File ChooseBedrockInput::sLastDirectory;

ChooseBedrockInput::ChooseBedrockInput(std::optional<B2JChooseInputState> state) {
  if (state) {
    fState = *state;
  }

  auto width = kWindowWidth;
  auto height = kWindowHeight;

  fBedrockGameDirectory = GameDirectory::BedrockSaveDirectory();

  setSize(width, height);
  {
    fMessage.reset(new Label("", TRANS("Select world to convert")));
    fMessage->setBounds(kMargin, kMargin, width - kMargin - kWorldListWidth - kMargin - kMargin, height - kMargin - kButtonBaseHeight - kMargin - kMargin);
    fMessage->setJustificationType(Justification::topLeft);
    fMessage->setMinimumHorizontalScale(1);
    addAndMakeVisible(*fMessage);
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
  {
    auto w = 160;
    fChooseCustomButton.reset(new TextButton(TRANS("Select mcworld file")));
    fChooseCustomButton->setBounds(width - kMargin - fNextButton->getWidth() - kMargin - w, height - kButtonBaseHeight - kMargin, w, kButtonBaseHeight);
    fChooseCustomButton->onClick = [this]() { onChooseCustomButtonClicked(); };
    addAndMakeVisible(*fChooseCustomButton);
  }

  Rectangle<int> listBoxBounds(width - kMargin - kWorldListWidth, kMargin, kWorldListWidth, height - 3 * kMargin - kButtonBaseHeight);
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
    fThread.reset(new GameDirectoryScanThreadBedrock(this));
    fThread->startThread();
  }
}

ChooseBedrockInput::~ChooseBedrockInput() {
  fListComponent.reset();
  fThread->stopThread(-1);
}

void ChooseBedrockInput::paint(juce::Graphics &g) {}

void ChooseBedrockInput::onNextButtonClicked() {
  JUCEApplication::getInstance()->invoke(gui::toB2JConfig, true);
}

void ChooseBedrockInput::onChooseCustomButtonClicked() {
  int flags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles;
  MainWindow::sFileChooser.reset(new FileChooser(TRANS("Select mcworld file to convert"), sLastDirectory, {}, true));
  MainWindow::sFileChooser->launchAsync(flags, [this](FileChooser const &chooser) { onCustomDirectorySelected(chooser); });
}

void ChooseBedrockInput::onCustomDirectorySelected(juce::FileChooser const &chooser) {
  File result = chooser.getResult();
  if (result == File()) {
    return;
  }
  fState.fInputFileOrDirectory = result;
  sLastDirectory = result.getParentDirectory();
  JUCEApplication::getInstance()->invoke(gui::toB2JConfig, true);
}

void ChooseBedrockInput::selectedRowsChanged(int lastRowSelected) {
  int num = fListComponent->getNumSelectedRows();
  if (num == 1 && 0 <= lastRowSelected && lastRowSelected < fGameDirectories.size()) {
    GameDirectory gd = fGameDirectories[lastRowSelected];
    fState.fInputFileOrDirectory = gd.fDirectory;
  } else {
    fState.fInputFileOrDirectory = std::nullopt;
  }
  if (fState.fInputFileOrDirectory != std::nullopt) {
    fNextButton->setEnabled(true);
  }
}

void ChooseBedrockInput::listBoxItemDoubleClicked(int row, const MouseEvent &) {
  if (row < 0 || fGameDirectories.size() <= row) {
    return;
  }
  GameDirectory gd = fGameDirectories[row];
  fState.fInputFileOrDirectory = gd.fDirectory;
  JUCEApplication::getInstance()->invoke(gui::toB2JConfig, false);
}

void ChooseBedrockInput::onBackButtonClicked() {
  JUCEApplication::getInstance()->invoke(gui::toModeSelect, true);
}

int ChooseBedrockInput::getNumRows() {
  return fGameDirectories.size();
}

void ChooseBedrockInput::paintListBoxItem(int rowNumber,
                                          juce::Graphics &g,
                                          int width, int height,
                                          bool rowIsSelected) {
  if (rowNumber < 0 || fGameDirectories.size() <= rowNumber) {
    return;
  }
  GameDirectory gd = fGameDirectories[rowNumber];
  gd.paint(g, width, height, rowIsSelected, *this);
}

void ChooseBedrockInput::handleAsyncUpdate() {
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

} // namespace je2be::gui::component
