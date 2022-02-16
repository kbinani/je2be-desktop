#include "B2JChooseInputComponent.h"
#include "CommandID.h"
#include "Constants.h"
#include "GameDirectories.h"
#include "MainWindow.h"

using namespace juce;

namespace juce {
Image juce_createIconForFile(const File &file);
}

namespace je2be::gui::b2j {

File B2JChooseInputComponent::sLastDirectory;

class B2JChooseInputComponent::GameDirectoryScanThread : public Thread {
public:
  std::vector<GameDirectory> fGameDirectories;
  B2JChooseInputComponent *const fOwner;

public:
  explicit GameDirectoryScanThread(B2JChooseInputComponent *owner) : Thread("je2be::gui::B2JChooseInputComponent::GameDirectoryScanThread"), fOwner(owner) {}

  void run() override {
    try {
      unsafeRun();
      fOwner->triggerAsyncUpdate();
    } catch (...) {
    }
  }

  void unsafeRun() {
    File dir = BedrockSaveDirectory();
    auto directories = dir.findChildFiles(File::findDirectories, false);
    for (File const &directory : directories) {
      File db = directory.getChildFile("db");
      if (threadShouldExit()) {
        break;
      }
      if (!db.isDirectory()) {
        continue;
      }
      File level = directory.getChildFile("level.dat");
      if (!level.existsAsFile()) {
        continue;
      }
      String levelName = directory.getFileName();
      File levelNameFile = directory.getChildFile("levelname.txt");
      if (levelNameFile.existsAsFile()) {
        StringArray lines;
        levelNameFile.readLines(lines);
        if (!lines.isEmpty()) {
          levelName = lines[0];
        }
      }
      B2JChooseInputComponent::GameDirectory gd;
      gd.fDirectory = directory;
      gd.fLevelName = levelName;
      fGameDirectories.push_back(gd);
    }
  }
};

B2JChooseInputComponent::B2JChooseInputComponent(std::optional<B2JChooseInputState> state) {
  if (state) {
    fState = *state;
  }

  auto width = kWindowWidth;
  auto height = kWindowHeight;
  auto fileListWidth = 280;

  fBedrockGameDirectory = BedrockSaveDirectory();

  setSize(width, height);
  {
    fMessage.reset(new Label("", TRANS("Select the world you want to convert")));
    fMessage->setBounds(kMargin, kMargin, width - kMargin - fileListWidth - kMargin - kMargin, height - kMargin - kButtonBaseHeight - kMargin - kMargin);
    fMessage->setJustificationType(Justification::topLeft);
    fMessage->setMinimumHorizontalScale(1);
    addAndMakeVisible(*fMessage);
  }
  {
    fBackButton.reset(new TextButton(TRANS("Back")));
    fBackButton->setBounds(kMargin, height - kMargin - kButtonBaseHeight, kButtonMinWidth, kButtonBaseHeight);
    fBackButton->setMouseCursor(MouseCursor::PointingHandCursor);
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
    fChooseCustomButton->setBounds(width - kMargin - fileListWidth, height - kButtonBaseHeight - kMargin, w, kButtonBaseHeight);
    fChooseCustomButton->setMouseCursor(MouseCursor::PointingHandCursor);
    fChooseCustomButton->onClick = [this]() { onChooseCustomButtonClicked(); };
    addAndMakeVisible(*fChooseCustomButton);
  }

  {
    fListComponent.reset(new ListBox("", this));
    fListComponent->setBounds(width - kMargin - fileListWidth, kMargin, fileListWidth, height - 3 * kMargin - kButtonBaseHeight);
    fListComponent->setEnabled(false);
    addAndMakeVisible(*fListComponent);
  }
  {
    fThread.reset(new GameDirectoryScanThread(this));
    fThread->startThread();
  }
}

B2JChooseInputComponent::~B2JChooseInputComponent() {
  fListComponent.reset();
  if (fThread->isThreadRunning()) {
    fThread->signalThreadShouldExit();
    fThread->wait(-1);
  }
}

void B2JChooseInputComponent::paint(juce::Graphics &g) {}

void B2JChooseInputComponent::onNextButtonClicked() {
  JUCEApplication::getInstance()->invoke(gui::toB2JConfig, true);
}

void B2JChooseInputComponent::onChooseCustomButtonClicked() {
  int flags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles;
  MainWindow::sFileChooser.reset(new FileChooser(TRANS("Select mcworld file to convert"), sLastDirectory, {}, false));
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
    fNextButton->setMouseCursor(MouseCursor::PointingHandCursor);
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

  if (rowIsSelected) {
    g.fillAll(findColour(DirectoryContentsDisplayComponent::highlightColourId));
  }

  const int x = 32;
  g.setColour(Colours::black);

  if (auto *d = getLookAndFeel().getDefaultFolderImage()) {
    d->drawWithin(g, Rectangle<float>(2.0f, 2.0f, x - 4.0f, (float)height - 4.0f),
                  RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize, 1.0f);
  }
  if (rowIsSelected) {
    g.setColour(findColour(DirectoryContentsDisplayComponent::highlightedTextColourId));
  } else {
    g.setColour(findColour(DirectoryContentsDisplayComponent::textColourId));
  }

  g.setFont((float)height * 0.7f);
  String name = gd.fLevelName + " [" + gd.fDirectory.getFileName() + "]";
  g.drawFittedText(name,
                   x, 0, width - x, height,
                   Justification::centredLeft, 1);
}

void B2JChooseInputComponent::handleAsyncUpdate() {
  fGameDirectories.swap(fThread->fGameDirectories);
  fListComponent->updateContent();
  fListComponent->setEnabled(true);
}

} // namespace je2be::gui::b2j
