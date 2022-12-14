#include <je2be.hpp>

#include "CommandID.h"
#include "Constants.h"
#include "File.h"
#include "GameDirectoryScanThreadXbox360.h"
#include "component/ChooseXbox360Input.h"
#include "component/MainWindow.h"
#include "component/SearchLabel.h"
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
    fSearch->onTextUpdate = [this]() {
      juce::String search = fSearch->getCurrentText();
      updateGameDirectoriesVisible();
      fSearchPlaceholder->setVisible(search.isEmpty());
    };
    fSearch->setWantsKeyboardFocus(false);
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
    fThread.reset(new GameDirectoryScanThreadXbox360(this));
    fThread->startThread();
  }
}

ChooseXbox360Input::~ChooseXbox360Input() {
  fListComponent.reset();
  fThread->signalThreadShouldExit();
  fThread->waitForThreadToExit(-1);
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
  juce::String worldName = GetWorldName(result, fGameDirectoriesAll);
  fState = ChooseInputState(InputType::Xbox360, result, worldName);
  sLastDirectory = result.getParentDirectory();
  JUCEApplication::getInstance()->invoke(fDestinationAfterChoose, true);
}

void ChooseXbox360Input::selectedRowsChanged(int lastRowSelected) {
  int num = fListComponent->getNumSelectedRows();
  if (num == 1 && 0 <= lastRowSelected && lastRowSelected < fGameDirectoriesVisible.size()) {
    GameDirectory gd = fGameDirectoriesVisible[lastRowSelected];
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
  if (row < 0 || fGameDirectoriesVisible.size() <= row) {
    return;
  }
  GameDirectory gd = fGameDirectoriesVisible[row];
  juce::String worldName = gd.fLevelName;
  fState = ChooseInputState(InputType::Xbox360, gd.fDirectory, worldName);
  JUCEApplication::getInstance()->invoke(fDestinationAfterChoose, false);
}

void ChooseXbox360Input::listBoxItemClicked(int row, MouseEvent const &e) {
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

void ChooseXbox360Input::onBackButtonClicked() {
  JUCEApplication::getInstance()->invoke(commands::toModeSelect, true);
}

int ChooseXbox360Input::getNumRows() {
  return fGameDirectoriesVisible.size();
}

void ChooseXbox360Input::paintListBoxItem(int rowNumber,
                                          juce::Graphics &g,
                                          int width, int height,
                                          bool rowIsSelected) {
  if (rowNumber < 0 || fGameDirectoriesVisible.size() <= rowNumber) {
    return;
  }
  GameDirectory gd = fGameDirectoriesVisible[rowNumber];
  gd.paint(g, width, height, rowIsSelected, *this, fSearch->getCurrentText());
}

void ChooseXbox360Input::handleAsyncUpdate() {
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

void ChooseXbox360Input::updateGameDirectoriesVisible() {
  juce::String search = fSearch->getCurrentText();
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
