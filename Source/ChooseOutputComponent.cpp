#include "ChooseOutputComponent.h"
#include "CommandID.h"
#include "Constants.h"
#include <JuceHeader.h>

ChooseOutputComponent::ChooseOutputComponent(ConvertState const &convertState)
    : fState(convertState), fListThread("j2b::gui::ChooseOutputComponent") {
  auto width = kWindowWidth;
  auto height = kWindowHeight;
  auto fileListWidth = 280;
  setSize(width, height);

  {
    fMessage.reset(new Label("", TRANS("Select a folder to save in")));
    fMessage->setBounds(
        kMargin, kMargin, width - kMargin - fileListWidth - kMargin - kMargin,
        height - kMargin - kButtonBaseHeight - kMargin - kMargin);
    fMessage->setJustificationType(Justification::topLeft);
    fMessage->setMinimumHorizontalScale(1);
    addAndMakeVisible(*fMessage);
  }
  {
    fSaveButton.reset(new TextButton(TRANS("Save")));
    fSaveButton->setBounds(width - kMargin - kButtonMinWidth,
                           height - kMargin - kButtonBaseHeight,
                           kButtonMinWidth, kButtonBaseHeight);
    fSaveButton->setEnabled(false);
    fSaveButton->onClick = [this]() { onSaveButtonClicked(); };
    addAndMakeVisible(*fSaveButton);
  }

  {
    fCancelButton.reset(new TextButton(TRANS("Cancel")));
    fCancelButton->setBounds(kMargin, height - kMargin - kButtonBaseHeight,
                             kButtonMinWidth, kButtonBaseHeight);
    fCancelButton->onClick = [this]() { onCancelButtonClicked(); };
    addAndMakeVisible(*fCancelButton);
  }

  fListThread.startThread();
  fList.reset(new DirectoryContentsList(nullptr, fListThread));

  File dir = File::getSpecialLocation(File::userApplicationDataDirectory)
                 .getParentDirectory()
                 .getChildFile("Local")
                 .getChildFile("Packages")
                 .getChildFile("Microsoft.MinecraftUWP_8wekyb3d8bbwe")
                 .getChildFile("LocalState")
                 .getChildFile("games")
                 .getChildFile("com.mojang")
                 .getChildFile("minecraftWorlds");

  fList->setDirectory(dir, true, false);

  {
    fListComponent.reset(new FileListComponent(*fList));
    fListComponent->setBounds(width - kMargin - fileListWidth, kMargin,
                              fileListWidth,
                              height - 3 * kMargin - kButtonBaseHeight);
    fListComponent->addListener(this);
    addAndMakeVisible(*fListComponent);
  }
}

ChooseOutputComponent::~ChooseOutputComponent() {
  fListComponent.reset();
  fList.reset();
}

void ChooseOutputComponent::paint(juce::Graphics &g) {}

void ChooseOutputComponent::selectionChanged() {
  int num = fListComponent->getNumSelectedFiles();
  if (num == 1) {
    fState.fCopyDestinationDirectory = fListComponent->getSelectedFile();
  } else {
    fState.fCopyDestinationDirectory = std::nullopt;
  }
  fSaveButton->setEnabled(fState.fCopyDestinationDirectory != std::nullopt);
}

void ChooseOutputComponent::fileClicked(const File &file, const MouseEvent &e) {
}

void ChooseOutputComponent::fileDoubleClicked(const File &file) {
  fState.fCopyDestinationDirectory = file;
  onSaveButtonClicked();
}

void ChooseOutputComponent::browserRootChanged(const File &newRoot) {}

void ChooseOutputComponent::onSaveButtonClicked() {
  if (!fState.fCopyDestinationDirectory) {
    return;
  }
  auto dest = *fState.fCopyDestinationDirectory;
  if (!dest.exists()) {
    return;
  }
  if (!dest.isDirectory()) {
    return;
  }
  bool containsSomething = false;
  RangedDirectoryIterator it(dest, false);
  for (auto const &e : it) {
    containsSomething = true;
    break;
  }
  if (containsSomething) {
    bool ok = NativeMessageBox::showOkCancelBox(
        AlertWindow::AlertIconType::QuestionIcon, TRANS("Confirmation"),
        TRANS("All files in the folder will be deleted and overwritten.\rThis "
              "process is irreversible.\rWould you like to continue?"));
    if (!ok) {
      return;
    }
  }
  JUCEApplication::getInstance()->perform({gui::toCopy});
}

void ChooseOutputComponent::onCancelButtonClicked() {
  JUCEApplication::getInstance()->perform({gui::toConfig});
}
