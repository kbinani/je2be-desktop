#include "component/b2j/B2JChooseOutput.h"
#include "CommandID.h"
#include "Constants.h"
#include "GameDirectory.h"
#include "component/MainWindow.h"

using namespace juce;

namespace je2be::gui::component::b2j {

static File DecideDefaultOutputDirectory(B2JConvertState const &s, File directory) {
  auto input = s.fConfigState.fInputState.fInputFileOrDirectory;
  String name = input->getFileName();
  if (input->isDirectory()) {
    File levelNameFile = input->getChildFile("levelname.txt");
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

File B2JChooseOutput::sLastCustomDirectory;
File B2JChooseOutput::sLastZipFile;

B2JChooseOutput::B2JChooseOutput(B2JConvertState const &convertState) : fState(convertState) {
  auto width = kWindowWidth;
  auto height = kWindowHeight;
  setSize(width, height);

  File root = GameDirectory::JavaSaveDirectory();
  fDefaultSaveDirectory = DecideDefaultOutputDirectory(convertState, root);

  int y = kMargin;
  fMessage.reset(new Label("", TRANS("Conversion completed! Choose how you want to save it")));
  fMessage->setBounds(kMargin, y, width - 2 * kMargin, kButtonBaseHeight);
  addAndMakeVisible(*fMessage);
  y += fMessage->getHeight();

  y += kMargin;
  fSaveToDefaultDirectory.reset(new component::TextButton(TRANS("Save into Minecraft Java edition save folder")));
  fSaveToDefaultDirectory->setBounds(2 * kMargin, y, width - 4 * kMargin, kButtonBaseHeight);
  fSaveToDefaultDirectory->setEnabled(root.exists());
  fSaveToDefaultDirectory->onClick = [this]() { onSaveToDefaultButtonClicked(); };
  addAndMakeVisible(*fSaveToDefaultDirectory);
  y += fSaveToDefaultDirectory->getHeight();

  y += kMargin;
  fSaveToCustomDirectory.reset(new component::TextButton(TRANS("Save into custom folder")));
  fSaveToCustomDirectory->setBounds(2 * kMargin, y, width - 4 * kMargin, kButtonBaseHeight);
  fSaveToCustomDirectory->onClick = [this]() { onSaveToCustomButtonClicked(); };
  addAndMakeVisible(*fSaveToCustomDirectory);
  y += fSaveToCustomDirectory->getHeight();

  {
    int w = 160;
    fBackButton.reset(new component::TextButton(TRANS("Back to the beginning")));
    fBackButton->setBounds(kMargin, height - kMargin - kButtonBaseHeight, w, kButtonBaseHeight);
    fBackButton->onClick = [this]() { onBackButtonClicked(); };
    addAndMakeVisible(*fBackButton);
  }

  fBackButton->setExplicitFocusOrder(2);
}

B2JChooseOutput::~B2JChooseOutput() {}

void B2JChooseOutput::onSaveToDefaultButtonClicked() {
  fState.fCopyDestination = fDefaultSaveDirectory;
  JUCEApplication::getInstance()->invoke(gui::toB2JCopy, true);
}

void B2JChooseOutput::onSaveToCustomButtonClicked() {
  if (sLastCustomDirectory == File()) {
    sLastCustomDirectory = GameDirectory::JavaSaveDirectory();
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

void B2JChooseOutput::onCustomDestinationDirectorySelected(FileChooser const &chooser) {
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

void B2JChooseOutput::paint(juce::Graphics &g) {}

void B2JChooseOutput::onBackButtonClicked() {
  JUCEApplication::getInstance()->invoke(gui::toB2JChooseInput, true);
}

} // namespace je2be::gui::component::b2j