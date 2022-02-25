#include "B2JChooseInputComponent.h"
#include "CommandID.h"
#include "Constants.h"
#include "GameDirectoryScanThreadBedrock.h"
#include "MainWindow.h"

using namespace juce;

namespace juce {
Image juce_createIconForFile(const File &file);
}

namespace je2be::gui::b2j {

File B2JChooseInputComponent::sLastDirectory;

B2JChooseInputComponent::B2JChooseInputComponent(std::optional<B2JChooseInputState> state) {
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
    fBackButton.reset(new TextButtonComponent(TRANS("Back")));
    fBackButton->setBounds(kMargin, height - kMargin - kButtonBaseHeight, kButtonMinWidth, kButtonBaseHeight);
    fBackButton->onClick = [this]() { onBackButtonClicked(); };
    addAndMakeVisible(*fBackButton);
  }
  {
    fNextButton.reset(new TextButtonComponent(TRANS("Next")));
    fNextButton->setBounds(width - kButtonMinWidth - kMargin, height - kButtonBaseHeight - kMargin, kButtonMinWidth, kButtonBaseHeight);
    fNextButton->onClick = [this]() { onNextButtonClicked(); };
    fNextButton->setEnabled(false);
    addAndMakeVisible(*fNextButton);
  }
  {
    auto w = 160;
    fChooseCustomButton.reset(new TextButtonComponent(TRANS("Select mcworld file")));
    fChooseCustomButton->setBounds(width - kMargin - fNextButton->getWidth() - kMargin - w, height - kButtonBaseHeight - kMargin, w, kButtonBaseHeight);
    fChooseCustomButton->onClick = [this]() { onChooseCustomButtonClicked(); };
    addAndMakeVisible(*fChooseCustomButton);
  }

  {
    fListComponent.reset(new ListBox("", this));
    fListComponent->setBounds(width - kMargin - kWorldListWidth, kMargin, kWorldListWidth, height - 3 * kMargin - kButtonBaseHeight);
    fListComponent->setEnabled(false);
    fListComponent->setRowHeight(kWorldListRowHeight);
    addAndMakeVisible(*fListComponent);
  }
  {
    fThread.reset(new GameDirectoryScanThreadBedrock(this));
    fThread->startThread();
  }
}

B2JChooseInputComponent::~B2JChooseInputComponent() {
  fListComponent.reset();
  fThread->stopThread(-1);
}

void B2JChooseInputComponent::paint(juce::Graphics &g) {}

void B2JChooseInputComponent::onNextButtonClicked() {
  JUCEApplication::getInstance()->invoke(gui::toB2JConfig, true);
}

void B2JChooseInputComponent::onChooseCustomButtonClicked() {
  int flags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles;
  MainWindow::sFileChooser.reset(new FileChooser(TRANS("Select mcworld file to convert"), sLastDirectory, {}, true));
  MainWindow::sFileChooser->launchAsync(flags, [this](FileChooser const &chooser) { onCustomDirectorySelected(chooser); });
}

void B2JChooseInputComponent::onCustomDirectorySelected(juce::FileChooser const &chooser) {
  File result = chooser.getResult();
  if (result == File()) {
    return;
  }
  fState.fInputFileOrDirectory = result;
  sLastDirectory = result.getParentDirectory();
  JUCEApplication::getInstance()->invoke(gui::toB2JConfig, true);
}

void B2JChooseInputComponent::selectedRowsChanged(int lastRowSelected) {
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

void B2JChooseInputComponent::listBoxItemDoubleClicked(int row, const MouseEvent &) {
  if (row < 0 || fGameDirectories.size() <= row) {
    return;
  }
  GameDirectory gd = fGameDirectories[row];
  fState.fInputFileOrDirectory = gd.fDirectory;
  JUCEApplication::getInstance()->invoke(gui::toB2JConfig, false);
}

void B2JChooseInputComponent::onBackButtonClicked() {
  JUCEApplication::getInstance()->invoke(gui::toModeSelect, true);
}

int B2JChooseInputComponent::getNumRows() {
  return fGameDirectories.size();
}

void B2JChooseInputComponent::paintListBoxItem(int rowNumber,
                                               juce::Graphics &g,
                                               int width, int height,
                                               bool rowIsSelected) {
  if (rowNumber < 0 || fGameDirectories.size() <= rowNumber) {
    return;
  }
  GameDirectory gd = fGameDirectories[rowNumber];
  gd.paint(g, width, height, rowIsSelected, *this);
}

void B2JChooseInputComponent::handleAsyncUpdate() {
  fGameDirectories.swap(fThread->fGameDirectories);
  fListComponent->updateContent();
  fListComponent->setEnabled(true);
}

} // namespace je2be::gui::b2j
