#include "component/ChooseJavaInput.h"
#include "CommandID.h"
#include "Constants.h"
#include "GameDirectoryScanThreadJava.h"
#include "component/MainWindow.h"
#include "component/TextButton.h"

using namespace juce;

namespace je2be::desktop::component {

File ChooseJavaInput::sLastDirectory;

ChooseJavaInput::ChooseJavaInput(std::optional<ChooseInputState> state) {
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
    fBackButton.reset(new component::TextButton(TRANS("Back")));
    fBackButton->setBounds(kMargin, height - kMargin - kButtonBaseHeight, kButtonMinWidth, kButtonBaseHeight);
    fBackButton->onClick = [this]() { onBackButtonClicked(); };
    addAndMakeVisible(*fBackButton);
  }
  {
    fNextButton.reset(new component::TextButton(TRANS("Next")));
    fNextButton->setBounds(width - kButtonMinWidth - kMargin, height - kButtonBaseHeight - kMargin, kButtonMinWidth, kButtonBaseHeight);
    fNextButton->onClick = [this]() { onNextButtonClicked(); };
    fNextButton->setEnabled(false);
    addAndMakeVisible(*fNextButton);
  }
  {
    auto w = 200;
    fChooseCustomButton.reset(new component::TextButton(TRANS("Select from other directories")));
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
    fThread.reset(new GameDirectoryScanThreadJava(this));
    fThread->startThread();
  }
}

ChooseJavaInput::~ChooseJavaInput() {
  fListComponent.reset();
}

void ChooseJavaInput::paint(juce::Graphics &g) {}

void ChooseJavaInput::onNextButtonClicked() {
  JUCEApplication::getInstance()->invoke(commands::toJ2BConfig, true);
}

void ChooseJavaInput::onChooseCustomButtonClicked() {
  if (sLastDirectory == File()) {
    sLastDirectory = GameDirectory::JavaSaveDirectory();
  }

  int flags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories;
  MainWindow::sFileChooser.reset(new FileChooser(TRANS("Select save data folder of Minecraft"), sLastDirectory, {}, true));
  MainWindow::sFileChooser->launchAsync(flags, [this](FileChooser const &chooser) { onCustomDirectorySelected(chooser); });
}

void ChooseJavaInput::onCustomDirectorySelected(juce::FileChooser const &chooser) {
  File result = chooser.getResult();
  if (result == File()) {
    return;
  }
  fState = ChooseInputState(InputType::Java, result, result.getFileName());
  sLastDirectory = result.getParentDirectory();
  JUCEApplication::getInstance()->invoke(commands::toJ2BConfig, true);
}

void ChooseJavaInput::selectedRowsChanged(int lastRowSelected) {
  int num = fListComponent->getNumSelectedRows();
  if (num == 1 && 0 <= lastRowSelected && lastRowSelected < fGameDirectories.size()) {
    GameDirectory gd = fGameDirectories[lastRowSelected];
    fState = ChooseInputState(InputType::Java, gd.fDirectory, gd.fDirectory.getFileName());
  } else {
    fState = std::nullopt;
  }
  if (fState != std::nullopt) {
    fNextButton->setEnabled(true);
  }
}

void ChooseJavaInput::listBoxItemDoubleClicked(int row, const MouseEvent &) {
  if (row < 0 || fGameDirectories.size() <= row) {
    return;
  }
  GameDirectory gd = fGameDirectories[row];
  fState = ChooseInputState(InputType::Java, gd.fDirectory, gd.fDirectory.getFileName());
  JUCEApplication::getInstance()->invoke(commands::toJ2BConfig, false);
}

void ChooseJavaInput::onBackButtonClicked() {
  JUCEApplication::getInstance()->invoke(commands::toModeSelect, true);
}

int ChooseJavaInput::getNumRows() {
  return fGameDirectories.size();
}

void ChooseJavaInput::paintListBoxItem(int rowNumber,
                                       juce::Graphics &g,
                                       int width, int height,
                                       bool rowIsSelected) {
  if (rowNumber < 0 || fGameDirectories.size() <= rowNumber) {
    return;
  }
  GameDirectory gd = fGameDirectories[rowNumber];
  gd.paint(g, width, height, rowIsSelected, *this);
}

void ChooseJavaInput::handleAsyncUpdate() {
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
