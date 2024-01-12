#include <je2be.hpp>

#include "CommandID.h"
#include "Constants.h"
#include "File.h"
#include "PS3GameDirectoryScanWorker.h"
#include "Thread.h"
#include "component/ChoosePS3Input.h"
#include "component/MainWindow.h"
#include "component/SearchLabel.h"
#include "component/TextButton.h"

using namespace juce;

namespace je2be::desktop::component {

File ChoosePS3Input::sLastDirectory;

ChoosePS3Input::ChoosePS3Input(juce::CommandID destinationAfterChoose, std::optional<ChooseInputState> state) : fDestinationAfterChoose(destinationAfterChoose) {
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
    fChooseCustomButton.reset(new TextButton(TRANS("Select GAMEDATA file")));
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

  int space = 5;
  juce::Rectangle<int> listBoxBounds(width - kMargin - kWorldListWidth, kMargin + kButtonBaseHeight + space, kWorldListWidth, height - 3 * kMargin - 2 * kButtonBaseHeight - space);
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

  juce::Rectangle<int> searchBounds(width - kMargin - kWorldListWidth, kMargin, kWorldListWidth, kButtonBaseHeight);
  {
    fSearch.reset(new SearchLabel());
    fSearch->setBounds(searchBounds);
    fSearch->setEnabled(true);
    fSearch->setEditable(true);
    fSearch->setColour(Label::ColourIds::backgroundColourId, fListComponent->findColour(ListBox::ColourIds::backgroundColourId));
    fSearch->onTextUpdate = [this]() { onSearchTextChanged(); };
    fSearch->onEditorHide = [this]() { onSearchTextChanged(); };
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
}

void ChoosePS3Input::parentHierarchyChanged() {
  if (fWorkerStarted) {
    return;
  }
  fWorkerStarted = true;
  auto worker = std::make_shared<PS3GameDirectoryScanWorker>(weak_from_this());
  fWorker = worker;
  Thread::Launch([worker]() {
    worker->run();
  });
}

ChoosePS3Input::~ChoosePS3Input() {
  fListComponent.reset();
  if (auto worker = fWorker.lock(); worker) {
    worker->signalThreadShouldExit();
  }
}

void ChoosePS3Input::paint(juce::Graphics &g) {}

void ChoosePS3Input::onNextButtonClicked() {
  JUCEApplication::getInstance()->invoke(fDestinationAfterChoose, true);
}

void ChoosePS3Input::onChooseCustomButtonClicked() {
  int flags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles;
  MainWindow::sFileChooser.reset(new FileChooser(TRANS("Select GAMEDATA file to convert"), sLastDirectory, "GAMEDATA", true));
  MainWindow::sFileChooser->launchAsync(flags, [this](FileChooser const &chooser) { onCustomDirectorySelected(chooser); });
}

void ChoosePS3Input::onCustomDirectorySelected(juce::FileChooser const &chooser) {
  File result = chooser.getResult();
  if (result == File()) {
    return;
  }
  auto parent = result.getParentDirectory();
  fState = ChooseInputState(InputType::PS3, parent, "world");
  sLastDirectory = parent;
  JUCEApplication::getInstance()->invoke(fDestinationAfterChoose, true);
}

void ChoosePS3Input::selectedRowsChanged(int lastRowSelected) {
  int num = fListComponent->getNumSelectedRows();
  if (num == 1 && 0 <= lastRowSelected && lastRowSelected < fGameDirectoriesVisible.size()) {
    GameDirectory gd = fGameDirectoriesVisible[lastRowSelected];
    juce::String worldName = gd.fLevelName;
    fState = ChooseInputState(InputType::PS3, gd.fDirectory, worldName);
  } else {
    fState = std::nullopt;
  }
  if (fState != std::nullopt) {
    fNextButton->setEnabled(true);
  }
}

void ChoosePS3Input::listBoxItemDoubleClicked(int row, const MouseEvent &) {
  if (row < 0 || fGameDirectoriesVisible.size() <= row) {
    return;
  }
  GameDirectory gd = fGameDirectoriesVisible[row];
  juce::String worldName = gd.fLevelName;
  fState = ChooseInputState(InputType::PS3, gd.fDirectory, worldName);
  JUCEApplication::getInstance()->invoke(fDestinationAfterChoose, false);
}

void ChoosePS3Input::listBoxItemClicked(int row, MouseEvent const &e) {
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
      if (!gd.fDirectory.existsAsFile()) {
        return;
      }
      gd.fDirectory.revealToUser();
    });
  }
}

void ChoosePS3Input::onBackButtonClicked() {
  JUCEApplication::getInstance()->invoke(commands::toModeSelect, true);
}

int ChoosePS3Input::getNumRows() {
  return fGameDirectoriesVisible.size();
}

void ChoosePS3Input::paintListBoxItem(int rowNumber,
                                      juce::Graphics &g,
                                      int width, int height,
                                      bool rowIsSelected) {
  if (rowNumber < 0 || fGameDirectoriesVisible.size() <= rowNumber) {
    return;
  }
  GameDirectory gd = fGameDirectoriesVisible[rowNumber];
  gd.paint(g, width, height, rowIsSelected, *this, fSearch->getCurrentText());
}

void ChoosePS3Input::handleAsyncUpdateWith(std::vector<GameDirectory> gameDirectories) {
  fGameDirectoriesAll = gameDirectories;
  if (fGameDirectoriesAll.empty()) {
    fPlaceholder->setText(TRANS("Nothing found in the save folder"), dontSendNotification);
  } else {
    onSearchTextChanged();
    fListComponent->setEnabled(true);
    fListComponent->setVisible(true);
    fPlaceholder->setText(TRANS("No matches found for search term"), dontSendNotification);
  }
}

void ChoosePS3Input::onSearchTextChanged() {
  if (fGameDirectoriesAll.empty()) {
    return;
  }

  juce::String search = fSearch->getCurrentText();
  fGameDirectoriesVisible.clear();
  std::copy_if(fGameDirectoriesAll.begin(), fGameDirectoriesAll.end(), std::back_inserter(fGameDirectoriesVisible), [search](GameDirectory const &gd) {
    return gd.match(search);
  });
  fListComponent->updateContent();
  for (int i = 0; i < fGameDirectoriesVisible.size(); i++) {
    fListComponent->repaintRow(i);
  }
  fSearchPlaceholder->setVisible(search.isEmpty());
  fPlaceholder->setVisible(fGameDirectoriesVisible.empty());
}

} // namespace je2be::desktop::component
