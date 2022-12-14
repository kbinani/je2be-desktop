#include "component/ChooseJavaInput.h"
#include "CommandID.h"
#include "Constants.h"
#include "GameDirectoryScanThreadJava.h"
#include "component/MainWindow.h"
#include "component/SearchLabel.h"
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

  int space = 5;
  Rectangle<int> listBoxBounds(width - kMargin - kWorldListWidth, kMargin + kButtonBaseHeight + space, kWorldListWidth, height - 3 * kMargin - 2 * kButtonBaseHeight - space);
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

  Rectangle<int> searchBounds(width - kMargin - kWorldListWidth, kMargin, kWorldListWidth, kButtonBaseHeight);
  {
    fSearch.reset(new SearchLabel());
    fSearch->setBounds(searchBounds);
    fSearch->setEnabled(true);
    fSearch->setEditable(true);
    fSearch->setColour(Label::ColourIds::backgroundColourId, fListComponent->findColour(ListBox::ColourIds::backgroundColourId));
    fSearch->onTextUpdate = [this]() {
      onSearchTextChanged();
    };
    fSearch->onEditorHide = [this]() {
      onSearchTextChanged();
    };
    addAndMakeVisible(*fSearch);
  }
  {
    fSearchPlaceholder.reset(new Label({}, TRANS("Search")));
    fSearchPlaceholder->setBounds(searchBounds);
    fSearchPlaceholder->setColour(Label::ColourIds::backgroundColourId, Colours::transparentWhite);
    fSearchPlaceholder->setEnabled(false);
    fSearchPlaceholder->setInterceptsMouseClicks(false, false);
    addAndMakeVisible(*fSearchPlaceholder);
  }

  {
    fThread.reset(new GameDirectoryScanThreadJava(this));
    fThread->startThread();
  }
}

ChooseJavaInput::~ChooseJavaInput() {
  fListComponent.reset();
  fThread->signalThreadShouldExit();
  fThread->waitForThreadToExit(-1);
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
  if (num == 1 && 0 <= lastRowSelected && lastRowSelected < fGameDirectoriesVisible.size()) {
    GameDirectory gd = fGameDirectoriesVisible[lastRowSelected];
    fState = ChooseInputState(InputType::Java, gd.fDirectory, gd.fDirectory.getFileName());
  } else {
    fState = std::nullopt;
  }
  if (fState != std::nullopt) {
    fNextButton->setEnabled(true);
  }
}

void ChooseJavaInput::listBoxItemDoubleClicked(int row, const MouseEvent &) {
  if (row < 0 || fGameDirectoriesVisible.size() <= row) {
    return;
  }
  GameDirectory gd = fGameDirectoriesVisible[row];
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
      if (row < 0 || fGameDirectoriesVisible.size() <= row) {
        return;
      }
      GameDirectory gd = fGameDirectoriesVisible[row];
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
  return fGameDirectoriesVisible.size();
}

void ChooseJavaInput::paintListBoxItem(int rowNumber,
                                       juce::Graphics &g,
                                       int width, int height,
                                       bool rowIsSelected) {
  if (rowNumber < 0 || fGameDirectoriesVisible.size() <= rowNumber) {
    return;
  }
  GameDirectory gd = fGameDirectoriesVisible[rowNumber];
  gd.paint(g, width, height, rowIsSelected, *this, fSearch->getCurrentText());
}

void ChooseJavaInput::handleAsyncUpdate() {
  fGameDirectoriesAll.swap(fThread->fGameDirectories);
  if (fGameDirectoriesAll.empty()) {
    fPlaceholder->setText(TRANS("Nothing found in the save folder"), dontSendNotification);
  } else {
    updateGameDirectoriesVisible();
    fListComponent->setEnabled(true);
    fListComponent->setVisible(true);
    fPlaceholder->setVisible(false);
  }
}

void ChooseJavaInput::onSearchTextChanged() {
  String search = fSearch->getCurrentText();
  updateGameDirectoriesVisible();
  fSearchPlaceholder->setVisible(search.isEmpty());
}

void ChooseJavaInput::updateGameDirectoriesVisible() {
  String search = fSearch->getCurrentText();
  fGameDirectoriesVisible.clear();
  std::copy_if(fGameDirectoriesAll.begin(), fGameDirectoriesAll.end(), std::back_inserter(fGameDirectoriesVisible), [search](GameDirectory const &gd) {
    return gd.match(search);
  });
  fListComponent->updateContent();
  for (int i = 0; i < fGameDirectoriesVisible.size(); i++) {
    fListComponent->repaintRow(i);
  }
}

} // namespace je2be::desktop::component
