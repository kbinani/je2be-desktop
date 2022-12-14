#include "component/ChooseBedrockInput.h"
#include "CommandID.h"
#include "Constants.h"
#include "File.h"
#include "GameDirectoryScanThreadBedrock.h"
#include "component/MainWindow.h"
#include "component/SearchLabel.h"
#include "component/TextButton.h"

using namespace juce;

namespace je2be::desktop::component {

File ChooseBedrockInput::sLastDirectory;

static String GetWorldName(File input) {
  String name = input.getFileName();
  if (input.isDirectory()) {
    File levelNameFile = input.getChildFile("levelname.txt");
    if (levelNameFile.existsAsFile()) {
      StringArray lines;
      levelNameFile.readLines(lines);
      if (!lines.isEmpty() && !lines[0].isEmpty()) {
        String line = lines[0];
        if (CharPointer_UTF8::isValidString(line.getCharPointer(), line.length())) {
          name = line;
        }
      }
    }
  } else {
    if (input.getFileExtension().isNotEmpty()) {
      name = input.getFileNameWithoutExtension();
    }
  }
  return name;
}

ChooseBedrockInput::ChooseBedrockInput(std::optional<ChooseInputState> state) {
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
    fOrMessage1.reset(new Label("", TRANS("or")));
    fOrMessage1->setBounds(kMargin + kMargin, y, fMessage->getWidth() - kMargin, lineHeight + borderSize.getTopAndBottom());
    fOrMessage1->setJustificationType(Justification::topLeft);
    addAndMakeVisible(*fOrMessage1);
    y += fOrMessage1->getHeight();

    y += kMargin;
    fChooseMcworldFileButton.reset(new TextButton(TRANS("Select *.mcworld file")));
    fChooseMcworldFileButton->setBounds(kMargin, y, fMessage->getWidth(), kButtonBaseHeight);
    fChooseMcworldFileButton->changeWidthToFitText();
    fChooseMcworldFileButton->setSize(jmin(fMessage->getWidth(), fChooseMcworldFileButton->getWidth() + 2 * kMargin), fChooseMcworldFileButton->getHeight());
    fChooseMcworldFileButton->onClick = [this]() { onChooseMcworldFileButtonClicked(); };
    addAndMakeVisible(*fChooseMcworldFileButton);
    y += fChooseMcworldFileButton->getHeight();

    y += kMargin;
    fOrMessage2.reset(new Label("", TRANS("or")));
    fOrMessage2->setBounds(kMargin + kMargin, y, fMessage->getWidth() - kMargin, lineHeight + borderSize.getTopAndBottom());
    fOrMessage2->setJustificationType(Justification::topLeft);
    addAndMakeVisible(*fOrMessage2);
    y += fOrMessage2->getHeight();

    y += kMargin;
    fChooseDirectoryButton.reset(new TextButton(TRANS("Select world folder")));
    fChooseDirectoryButton->setBounds(kMargin, y, fMessage->getWidth(), kButtonBaseHeight);
    fChooseDirectoryButton->changeWidthToFitText();
    fChooseDirectoryButton->setSize(jmin(fMessage->getWidth(), fChooseDirectoryButton->getWidth() + 2 * kMargin), fChooseDirectoryButton->getHeight());
    fChooseDirectoryButton->onClick = [this]() { onChooseDirectoryButtonClicked(); };
    addAndMakeVisible(*fChooseDirectoryButton);
    y += fChooseDirectoryButton->getHeight();
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

  {
    fThread.reset(new GameDirectoryScanThreadBedrock(this));
    fThread->startThread();
  }
}

ChooseBedrockInput::~ChooseBedrockInput() {
  fListComponent.reset();
  fThread->signalThreadShouldExit();
  fThread->waitForThreadToExit(-1);
}

void ChooseBedrockInput::paint(juce::Graphics &g) {}

void ChooseBedrockInput::onNextButtonClicked() {
  JUCEApplication::getInstance()->invoke(commands::toB2JConfig, true);
}

void ChooseBedrockInput::onChooseMcworldFileButtonClicked() {
  int flags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles;
  MainWindow::sFileChooser.reset(new FileChooser(TRANS("Select *.mcworld file to convert"), sLastDirectory, "*.mcworld", true));
  MainWindow::sFileChooser->launchAsync(flags, [this](FileChooser const &chooser) { onMcworldFileSelected(chooser); });
}

void ChooseBedrockInput::onChooseDirectoryButtonClicked() {
  int flags = FileBrowserComponent::canSelectDirectories;
  MainWindow::sFileChooser.reset(new FileChooser(TRANS("Select world folder to convert"), sLastDirectory, {}, true));
  MainWindow::sFileChooser->launchAsync(flags, [this](FileChooser const &chooser) { onDirectorySelected(chooser); });
}

void ChooseBedrockInput::onMcworldFileSelected(juce::FileChooser const &chooser) {
  File result = chooser.getResult();
  if (result == File()) {
    return;
  }
  String worldName = GetWorldName(result);
  fState = ChooseInputState(InputType::Bedrock, result, worldName);
  sLastDirectory = result.getParentDirectory();
  JUCEApplication::getInstance()->invoke(commands::toB2JConfig, true);
}

void ChooseBedrockInput::onDirectorySelected(juce::FileChooser const &chooser) {
  File result = chooser.getResult();
  if (result == File()) {
    return;
  }
  String worldName = GetWorldName(result);
  fState = ChooseInputState(InputType::Bedrock, result, worldName);
  sLastDirectory = result;
  JUCEApplication::getInstance()->invoke(commands::toB2JConfig, true);
}

void ChooseBedrockInput::selectedRowsChanged(int lastRowSelected) {
  int num = fListComponent->getNumSelectedRows();
  if (num == 1 && 0 <= lastRowSelected && lastRowSelected < fGameDirectoriesVisible.size()) {
    GameDirectory gd = fGameDirectoriesVisible[lastRowSelected];
    String worldName = GetWorldName(gd.fDirectory);
    fState = ChooseInputState(InputType::Bedrock, gd.fDirectory, worldName);
  } else {
    fState = std::nullopt;
  }
  if (fState != std::nullopt) {
    fNextButton->setEnabled(true);
  }
}

void ChooseBedrockInput::listBoxItemDoubleClicked(int row, MouseEvent const &) {
  if (row < 0 || fGameDirectoriesVisible.size() <= row) {
    return;
  }
  GameDirectory gd = fGameDirectoriesVisible[row];
  String worldName = GetWorldName(gd.fDirectory);
  fState = ChooseInputState(InputType::Bedrock, gd.fDirectory, worldName);
  JUCEApplication::getInstance()->invoke(commands::toB2JConfig, false);
}

void ChooseBedrockInput::listBoxItemClicked(int row, MouseEvent const &e) {
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

void ChooseBedrockInput::onBackButtonClicked() {
  JUCEApplication::getInstance()->invoke(commands::toModeSelect, true);
}

int ChooseBedrockInput::getNumRows() {
  return fGameDirectoriesVisible.size();
}

void ChooseBedrockInput::paintListBoxItem(int rowNumber,
                                          juce::Graphics &g,
                                          int width, int height,
                                          bool rowIsSelected) {
  if (rowNumber < 0 || fGameDirectoriesVisible.size() <= rowNumber) {
    return;
  }
  GameDirectory gd = fGameDirectoriesVisible[rowNumber];
  gd.paint(g, width, height, rowIsSelected, *this, fSearch->getCurrentText());
}

void ChooseBedrockInput::handleAsyncUpdate() {
  fGameDirectoriesAll.swap(fThread->fGameDirectories);
  if (fGameDirectoriesAll.empty()) {
    fPlaceholder->setText(TRANS("Nothing found in the save folder"), dontSendNotification);
  } else {
    onSearchTextChanged();
    fListComponent->setEnabled(true);
    fListComponent->setVisible(true);
    fPlaceholder->setText(TRANS("No matches found for search term"), dontSendNotification);
  }
}

void ChooseBedrockInput::onSearchTextChanged() {
  if (fGameDirectoriesAll.empty()) {
    return;
  }

  String search = fSearch->getCurrentText();
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
