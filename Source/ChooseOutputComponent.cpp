#include "ChooseOutputComponent.h"
#include "CommandID.h"
#include "Constants.h"
#include <JuceHeader.h>

ChooseOutputComponent::ChooseOutputComponent(ConvertState const &convertState)
    : fState(convertState) {
  auto width = kWindowWidth;
  auto height = kWindowHeight;
  auto fileListWidth = 280;
  setSize(width, height);

  {
    fMessage.reset(
        new Label("", TRANS("Select a folder to save in.\rChoose an empty "
                            "folder to protect your existing data.")));
    fMessage->setBounds(kMargin, kMargin, width - 2 * kMargin,
                        height - kMargin - kButtonBaseHeight - kMargin -
                            kMargin);
    fMessage->setJustificationType(Justification::topLeft);
    fMessage->setMinimumHorizontalScale(1);
    addAndMakeVisible(*fMessage);
  }
  {
    int w = 160;
    fBackButton.reset(new TextButton(TRANS("Back to the beginning")));
    fBackButton->setBounds(kMargin, height - kMargin - kButtonBaseHeight, w,
                           kButtonBaseHeight);
    fBackButton->onClick = [this]() { onBackButtonClicked(); };
    addAndMakeVisible(*fBackButton);
  }
  {
    fBrowseButton.reset(new TextButton(TRANS("Choose again")));
    fBrowseButton->setBounds(width - kMargin - kButtonMinWidth,
                             height - kMargin - kButtonBaseHeight,
                             kButtonMinWidth, kButtonBaseHeight);
    fBrowseButton->onClick = [this]() { onBrowseButtonClicked(); };
    addAndMakeVisible(*fBrowseButton);
  }
  triggerAsyncUpdate();
}

ChooseOutputComponent::~ChooseOutputComponent() {}

void ChooseOutputComponent::onBrowseButtonClicked() {
  static File lastDir =
      File::getSpecialLocation(File::userApplicationDataDirectory)
          .getParentDirectory()
          .getChildFile("Local")
          .getChildFile("Packages")
          .getChildFile("Microsoft.MinecraftUWP_8wekyb3d8bbwe")
          .getChildFile("LocalState")
          .getChildFile("games")
          .getChildFile("com.mojang")
          .getChildFile("minecraftWorlds");

  FileChooser chooser(TRANS("Select an empty folder to save in"), lastDir);
  bool ok = chooser.browseForDirectory();
  if (!ok) {
    return;
  }
  File dest = chooser.getResult();
  RangedDirectoryIterator it(dest, false);
  bool containsSomething = false;
  for (auto const &e : it) {
    containsSomething = true;
    break;
  }
  if (containsSomething) {
    NativeMessageBox::showMessageBox(
        AlertWindow::AlertIconType::WarningIcon, TRANS("Error"),
        TRANS("There are files and folders in the directory.\rPlease select an "
              "empty folder"));
  } else {
    fState.fCopyDestinationDirectory = dest;
    JUCEApplication::getInstance()->perform({gui::toCopy});
  }
}

void ChooseOutputComponent::paint(juce::Graphics &g) {}

void ChooseOutputComponent::handleAsyncUpdate() { onBrowseButtonClicked(); }

void ChooseOutputComponent::onBackButtonClicked() {
  JUCEApplication::getInstance()->perform({gui::toChooseInput});
}
