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
    fChooseCustomButton.reset(new component::TextButton(TRANS("Select from other directories")));
    fChooseCustomButton->setBounds(kMargin, y, fMessage->getWidth(), kButtonBaseHeight);
    fChooseCustomButton->changeWidthToFitText();
    fChooseCustomButton->setSize(jmin(fMessage->getWidth(), fChooseCustomButton->getWidth() + 2 * kMargin), fChooseCustomButton->getHeight());
    fChooseCustomButton->onClick = [this]() { onChooseCustomButtonClicked(); };
    addAndMakeVisible(*fChooseCustomButton);
    y += fChooseCustomButton->getHeight();
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

void ChooseJavaInput::listBoxItemClicked(int row, MouseEvent const &e) {
  if (e.mods.isRightButtonDown()) {
    PopupMenu menu;
    menu.addItem(1, TRANS("Show in Explorer"), true, false, nullptr);
    PopupMenu::Options o;
    menu.showMenuAsync(o, [this, row](int result) {
      if (result != 1) {
        return;
      }
      if (row < 0 || fGameDirectories.size() <= row) {
        return;
      }
      GameDirectory gd = fGameDirectories[row];
      if (!gd.fDirectory.isDirectory()) {
        return;
      }
      gd.fDirectory.revealToUser();
    });
  }
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
