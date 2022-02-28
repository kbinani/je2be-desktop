#include "component/b2j/B2JChooseInput.h"
#include "CommandID.h"
#include "Constants.h"
#include "GameDirectoryScanThreadBedrock.h"
#include "component/MainWindow.h"
#include "component/TextButton.h"

using namespace juce;

namespace je2be::gui::component::b2j {

File B2JChooseInput::sLastDirectory;

B2JChooseInput::B2JChooseInput(std::optional<B2JChooseInputState> state) {
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

B2JChooseInput::~B2JChooseInput() {
  fListComponent.reset();
  fThread->stopThread(-1);
}

void B2JChooseInput::paint(juce::Graphics &g) {}

void B2JChooseInput::onNextButtonClicked() {
  JUCEApplication::getInstance()->invoke(gui::toB2JConfig, true);
}

void B2JChooseInput::onChooseCustomButtonClicked() {
  int flags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles;
  MainWindow::sFileChooser.reset(new FileChooser(TRANS("Select mcworld file to convert"), sLastDirectory, {}, true));
  MainWindow::sFileChooser->launchAsync(flags, [this](FileChooser const &chooser) { onCustomDirectorySelected(chooser); });
}

void B2JChooseInput::onCustomDirectorySelected(juce::FileChooser const &chooser) {
  File result = chooser.getResult();
  if (result == File()) {
    return;
  }
  fState.fInputFileOrDirectory = result;
  sLastDirectory = result.getParentDirectory();
  JUCEApplication::getInstance()->invoke(gui::toB2JConfig, true);
}

void B2JChooseInput::selectedRowsChanged(int lastRowSelected) {
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

void B2JChooseInput::listBoxItemDoubleClicked(int row, const MouseEvent &) {
  if (row < 0 || fGameDirectories.size() <= row) {
    return;
  }
  GameDirectory gd = fGameDirectories[row];
  fState.fInputFileOrDirectory = gd.fDirectory;
  JUCEApplication::getInstance()->invoke(gui::toB2JConfig, false);
}

void B2JChooseInput::onBackButtonClicked() {
  JUCEApplication::getInstance()->invoke(gui::toModeSelect, true);
}

int B2JChooseInput::getNumRows() {
  return fGameDirectories.size();
}

void B2JChooseInput::paintListBoxItem(int rowNumber,
                                      juce::Graphics &g,
                                      int width, int height,
                                      bool rowIsSelected) {
  if (rowNumber < 0 || fGameDirectories.size() <= rowNumber) {
    return;
  }
  GameDirectory gd = fGameDirectories[rowNumber];
  gd.paint(g, width, height, rowIsSelected, *this);
}

void B2JChooseInput::handleAsyncUpdate() {
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

} // namespace je2be::gui::component::b2j
