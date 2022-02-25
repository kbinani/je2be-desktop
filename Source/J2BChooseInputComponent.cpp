#include "J2BChooseInputComponent.h"
#include "CommandID.h"
#include "Constants.h"
#include "GameDirectory.h"
#include "MainWindow.h"

using namespace juce;

namespace je2be::gui::j2b {

File J2BChooseInputComponent::sLastDirectory;

J2BChooseInputComponent::J2BChooseInputComponent(std::optional<J2BChooseInputState> state) {
  if (state) {
    fState = *state;
  }

  auto width = kWindowWidth;
  auto height = kWindowHeight;

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
    fChooseCustomButton.reset(new TextButtonComponent(TRANS("Select from other directories")));
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
    fThread.reset(new GameDirectoryScanThreadJava(this));
    fThread->startThread();
  }
}

J2BChooseInputComponent::~J2BChooseInputComponent() {
  fListComponent.reset();
}

void J2BChooseInputComponent::paint(juce::Graphics &g) {}

void J2BChooseInputComponent::onNextButtonClicked() {
  JUCEApplication::getInstance()->invoke(gui::toJ2BConfig, true);
}

void J2BChooseInputComponent::onChooseCustomButtonClicked() {
  if (sLastDirectory == File()) {
    sLastDirectory = GameDirectory::JavaSaveDirectory();
  }

  int flags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories;
  MainWindow::sFileChooser.reset(new FileChooser(TRANS("Select save data folder of Minecraft"), sLastDirectory, {}, true));
  MainWindow::sFileChooser->launchAsync(flags, [this](FileChooser const &chooser) { onCustomDirectorySelected(chooser); });
}

void J2BChooseInputComponent::onCustomDirectorySelected(juce::FileChooser const &chooser) {
  File result = chooser.getResult();
  if (result == File()) {
    return;
  }
  fState.fInputDirectory = result;
  sLastDirectory = result.getParentDirectory();
  JUCEApplication::getInstance()->invoke(gui::toJ2BConfig, true);
}

void J2BChooseInputComponent::selectedRowsChanged(int lastRowSelected) {
  int num = fListComponent->getNumSelectedRows();
  if (num == 1 && 0 <= lastRowSelected && lastRowSelected < fGameDirectories.size()) {
    GameDirectory gd = fGameDirectories[lastRowSelected];
    fState.fInputDirectory = gd.fDirectory;
  } else {
    fState.fInputDirectory = std::nullopt;
  }
  if (fState.fInputDirectory != std::nullopt) {
    fNextButton->setEnabled(true);
  }
}

void J2BChooseInputComponent::listBoxItemDoubleClicked(int row, const MouseEvent &) {
  if (row < 0 || fGameDirectories.size() <= row) {
    return;
  }
  GameDirectory gd = fGameDirectories[row];
  fState.fInputDirectory = gd.fDirectory;
  JUCEApplication::getInstance()->invoke(gui::toJ2BConfig, false);
}

void J2BChooseInputComponent::onBackButtonClicked() {
  JUCEApplication::getInstance()->invoke(gui::toModeSelect, true);
}

int J2BChooseInputComponent::getNumRows() {
  return fGameDirectories.size();
}

void J2BChooseInputComponent::paintListBoxItem(int rowNumber,
                                               juce::Graphics &g,
                                               int width, int height,
                                               bool rowIsSelected) {
  if (rowNumber < 0 || fGameDirectories.size() <= rowNumber) {
    return;
  }
  GameDirectory gd = fGameDirectories[rowNumber];
  gd.paint(g, width, height, rowIsSelected, *this);
}

void J2BChooseInputComponent::handleAsyncUpdate() {
  fGameDirectories.swap(fThread->fGameDirectories);
  fListComponent->updateContent();
  fListComponent->setEnabled(true);
}

} // namespace je2be::gui::j2b
