#include "B2JChooseOutputComponent.h"
#include "CommandID.h"
#include "Constants.h"
#include "GameDirectories.h"
#include "MainWindow.h"

using namespace juce;

namespace je2be::gui {

static File DecideDefaultOutputDirectory(B2JConvertState const &s, File directory) {
  auto input = s.fConfigState.fInputState.fInputFileOrDirectory;
  String name = input->getFileName();
  if (input->isDirectory()) {
    File levelNameFile = input->getChildFile("levelname.txt");
    if (levelNameFile.existsAsFile()) {
      StringArray lines;
      levelNameFile.readLines(lines);
      if (!lines.isEmpty() && !lines[0].isEmpty()) {
        name = lines[0];
      }
    }
  } else {
    if (input->getFileExtension().isNotEmpty()) {
      name = input->getFileNameWithoutExtension();
    }
  }
  File candidate = directory.getChildFile(name);
  int count = 0;
  while (candidate.exists()) {
    count++;
    candidate = directory.getChildFile(name + "-" + String(count));
  }
  return candidate;
}

File B2JChooseOutputComponent::sLastCustomDirectory;
File B2JChooseOutputComponent::sLastZipFile;

B2JChooseOutputComponent::B2JChooseOutputComponent(B2JConvertState const &convertState) : fState(convertState) {
  auto width = kWindowWidth;
  auto height = kWindowHeight;
  setSize(width, height);

  File root = BedrockSaveDirectory();
  fDefaultSaveDirectory = DecideDefaultOutputDirectory(convertState, root);

  int y = kMargin;
  fMessage.reset(new Label("", TRANS("Conversion completed! Choose how you want to save it")));
  fMessage->setBounds(kMargin, y, width - 2 * kMargin, kButtonBaseHeight);
  addAndMakeVisible(*fMessage);
  y += fMessage->getHeight();

  y += kMargin;
  fSaveToDefaultDirectory.reset(new TextButton(TRANS("Save into Minecraft Java edition save folder")));
  fSaveToDefaultDirectory->setBounds(2 * kMargin, y, width - 4 * kMargin, kButtonBaseHeight);
  fSaveToDefaultDirectory->setEnabled(root.exists());
  if (root.exists()) {
    fSaveToDefaultDirectory->setMouseCursor(MouseCursor::PointingHandCursor);
  }
  fSaveToDefaultDirectory->onClick = [this]() { onSaveToDefaultButtonClicked(); };
  addAndMakeVisible(*fSaveToDefaultDirectory);
  y += fSaveToDefaultDirectory->getHeight();

  y += kMargin;
  fSaveToCustomDirectory.reset(new TextButton(TRANS("Save into custom folder")));
  fSaveToCustomDirectory->setBounds(2 * kMargin, y, width - 4 * kMargin, kButtonBaseHeight);
  fSaveToCustomDirectory->setMouseCursor(MouseCursor::PointingHandCursor);
  fSaveToCustomDirectory->onClick = [this]() { onSaveToCustomButtonClicked(); };
  addAndMakeVisible(*fSaveToCustomDirectory);
  y += fSaveToCustomDirectory->getHeight();

  {
    int w = 160;
    fBackButton.reset(new TextButton(TRANS("Back to the beginning")));
    fBackButton->setBounds(kMargin, height - kMargin - kButtonBaseHeight, w, kButtonBaseHeight);
    fBackButton->setMouseCursor(MouseCursor::PointingHandCursor);
    fBackButton->onClick = [this]() { onBackButtonClicked(); };
    addAndMakeVisible(*fBackButton);
  }

  fBackButton->setExplicitFocusOrder(2);
}

B2JChooseOutputComponent::~B2JChooseOutputComponent() {}

void B2JChooseOutputComponent::onSaveToDefaultButtonClicked() {
  fState.fCopyDestination = fDefaultSaveDirectory;
  JUCEApplication::getInstance()->invoke(gui::toB2JCopy, true);
}

void B2JChooseOutputComponent::onSaveToCustomButtonClicked() {
  if (sLastCustomDirectory == File()) {
    sLastCustomDirectory = JavaSaveDirectory();
  }

  File directory = sLastCustomDirectory;
  if (auto defaultDirectory = DecideDefaultOutputDirectory(fState.fConvertState, sLastCustomDirectory); defaultDirectory != File()) {
    directory = defaultDirectory;
  }

  auto chooser = new FileChooser(TRANS("Select an empty folder to save in"), directory, {}, false);
  MainWindow::sFileChooser.reset(chooser);
  int flags = FileBrowserComponent::saveMode | FileBrowserComponent::canSelectDirectories;
  MainWindow::sFileChooser->launchAsync(flags, [this](FileChooser const &chooser) { onCustomDestinationDirectorySelected(chooser); });
}

void B2JChooseOutputComponent::onCustomDestinationDirectorySelected(FileChooser const &chooser) {
  File dest = chooser.getResult();
  if (dest == File()) {
    fSaveToCustomDirectory->setToggleState(false, dontSendNotification);
    fState.fCopyDestination = std::nullopt;
    return;
  }
  sLastCustomDirectory = dest;
  RangedDirectoryIterator it(dest, false);
  bool containsSomething = false;
  for (auto const &e : it) {
    containsSomething = true;
    break;
  }
  if (containsSomething) {
    fSaveToCustomDirectory->setToggleState(false, dontSendNotification);
    fState.fCopyDestination = std::nullopt;
    NativeMessageBox::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon, TRANS("Error"),
                                          TRANS("There are files and folders in the directory.\rPlease select an "
                                                "empty folder"));
  } else {
    fState.fCopyDestination = dest;
    JUCEApplication::getInstance()->invoke(gui::toB2JCopy, true);
  }
}

void B2JChooseOutputComponent::paint(juce::Graphics &g) {}

void B2JChooseOutputComponent::onBackButtonClicked() {
  JUCEApplication::getInstance()->invoke(gui::toB2JChooseInput, true);
}

} // namespace je2be::gui
