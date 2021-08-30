#include "ChooseOutputComponent.h"
#include "CommandID.h"
#include "Constants.h"

using namespace juce;

static File BedrockSaveDirectory() {
  return File::getSpecialLocation(File::userApplicationDataDirectory)
      .getParentDirectory()
      .getChildFile("Local")
      .getChildFile("Packages")
      .getChildFile("Microsoft.MinecraftUWP_8wekyb3d8bbwe")
      .getChildFile("LocalState")
      .getChildFile("games")
      .getChildFile("com.mojang")
      .getChildFile("minecraftWorlds");
}

static File DecideDefaultOutputDirectory(ConvertState const &s) {
  File root = BedrockSaveDirectory();
  String name = s.fConfigState.fInputState.fInputDirectory->getFileName();
  File candidate = root.getChildFile(name);
  int count = 0;
  while (candidate.exists()) {
    count++;
    candidate = root.getChildFile(name + "-" + String(count));
  }
  return candidate;
}

File ChooseOutputComponent::sLastCustomDirectory;
File ChooseOutputComponent::sLastZipFile;

ChooseOutputComponent::ChooseOutputComponent(ConvertState const &convertState)
    : fState(convertState) {
  auto width = kWindowWidth;
  auto height = kWindowHeight;
  setSize(width, height);

  File root = BedrockSaveDirectory();
  fDefaultSaveDirectory = DecideDefaultOutputDirectory(convertState);

  int y = kMargin;
  fMessage.reset(new Label(
      "", TRANS("Conversion completed! Choose how you want to save it")));
  fMessage->setBounds(kMargin, y, width - 2 * kMargin, kButtonBaseHeight);
  addAndMakeVisible(*fMessage);
  y += fMessage->getHeight();

  y += kMargin;
  fSaveToDefaultDirectory.reset(
      new TextButton(TRANS("Save into Minecraft Windows 10 save folder")));
  fSaveToDefaultDirectory->setBounds(2 * kMargin, y, width - 4 * kMargin,
                                     kButtonBaseHeight);
  fSaveToDefaultDirectory->setEnabled(root.exists());
  if (root.exists()) {
    fSaveToDefaultDirectory->setMouseCursor(MouseCursor::PointingHandCursor);
  }
  fSaveToDefaultDirectory->onClick = [this]() {
    onSaveToDefaultButtonClicked();
  };
  addAndMakeVisible(*fSaveToDefaultDirectory);
  y += fSaveToDefaultDirectory->getHeight();

  y += kMargin;
  fSaveToCustomDirectory.reset(
      new TextButton(TRANS("Save into custom folder")));
  fSaveToCustomDirectory->setBounds(2 * kMargin, y, width - 4 * kMargin,
                                    kButtonBaseHeight);
  fSaveToCustomDirectory->setMouseCursor(MouseCursor::PointingHandCursor);
  fSaveToCustomDirectory->onClick = [this]() { onSaveToCustomButtonClicked(); };
  addAndMakeVisible(*fSaveToCustomDirectory);
  y += fSaveToCustomDirectory->getHeight();

  y += kMargin;
  fSaveAsZipFile.reset(new TextButton(TRANS("Export as *.mcworld file")));
  fSaveAsZipFile->setBounds(2 * kMargin, y, width - 4 * kMargin,
                            kButtonBaseHeight);
  fSaveAsZipFile->setMouseCursor(MouseCursor::PointingHandCursor);
  fSaveAsZipFile->onClick = [this]() { onSaveAsZipButtonClicked(); };
  addAndMakeVisible(*fSaveAsZipFile);
  y += fSaveAsZipFile->getHeight();

  {
    int w = 160;
    fBackButton.reset(new TextButton(TRANS("Back to the beginning")));
    fBackButton->setBounds(kMargin, height - kMargin - kButtonBaseHeight, w,
                           kButtonBaseHeight);
    fBackButton->setMouseCursor(MouseCursor::PointingHandCursor);
    fBackButton->onClick = [this]() { onBackButtonClicked(); };
    addAndMakeVisible(*fBackButton);
  }

  fBackButton->setExplicitFocusOrder(2);
}

ChooseOutputComponent::~ChooseOutputComponent() {}

void ChooseOutputComponent::onSaveToDefaultButtonClicked() {
  fState.fCopyDestination = fDefaultSaveDirectory;
  fState.fFormat = OutputFormat::Directory;
  JUCEApplication::getInstance()->invoke(gui::toCopy, true);
}

void ChooseOutputComponent::onSaveToCustomButtonClicked() {
  if (sLastCustomDirectory == File()) {
    sLastCustomDirectory = BedrockSaveDirectory();
  }

  fFileChooser.reset(new FileChooser(TRANS("Select an empty folder to save in"),
                                     sLastCustomDirectory));
  int flags = FileBrowserComponent::openMode |
              FileBrowserComponent::canSelectDirectories;
  fFileChooser->launchAsync(flags, [this](FileChooser const &chooser) {
    onCustomDestinationDirectorySelected(chooser);
  });
}

void ChooseOutputComponent::onCustomDestinationDirectorySelected(
    FileChooser const &chooser) {
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
    NativeMessageBox::showMessageBoxAsync(
        AlertWindow::AlertIconType::WarningIcon, TRANS("Error"),
        TRANS("There are files and folders in the directory.\rPlease select an "
              "empty folder"));
  } else {
    fState.fCopyDestination = dest;
    fState.fFormat = OutputFormat::Directory;
    JUCEApplication::getInstance()->invoke(gui::toCopy, true);
  }
}

void ChooseOutputComponent::onSaveAsZipButtonClicked() {
  fFileChooser.reset(new FileChooser(TRANS("Choose where to export the file"),
                                     sLastZipFile, "*.mcworld"));
  int flags = FileBrowserComponent::saveMode |
              FileBrowserComponent::canSelectFiles |
              FileBrowserComponent::warnAboutOverwriting;
  fFileChooser->launchAsync(flags, [this](FileChooser const &chooser) {
    onZipDestinationFileSelected(chooser);
  });
}

void ChooseOutputComponent::onZipDestinationFileSelected(
    FileChooser const &chooser) {
  File dest = chooser.getResult();
  if (dest == File()) {
    fSaveAsZipFile->setToggleState(false, dontSendNotification);
    fState.fCopyDestination = std::nullopt;
    return;
  }
  sLastZipFile = dest;
  fState.fCopyDestination = dest;
  fState.fFormat = OutputFormat::MCWorld;
  JUCEApplication::getInstance()->invoke(gui::toCopy, true);
}

void ChooseOutputComponent::paint(juce::Graphics &g) {}

void ChooseOutputComponent::onBackButtonClicked() {
  JUCEApplication::getInstance()->invoke(gui::toChooseInput, true);
}
