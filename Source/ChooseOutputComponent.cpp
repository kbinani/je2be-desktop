#include "ChooseOutputComponent.h"
#include "CommandID.h"
#include "Constants.h"
#include <JuceHeader.h>

ChooseOutputComponent::ChooseOutputComponent(ConvertState const &convertState)
    : fState(convertState), fListThread("j2b::gui::ChooseOutputComponent") {
  auto width = kWindowWidth;
  auto height = kWindowHeight;
  setSize(width, height);

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
    auto w = 360;
    fListComponent.reset(new FileListComponent(*fList));
    fListComponent->setBounds(width - kMargin - w, kMargin, w,
                              height - 3 * kMargin - kButtonBaseHeight);
    fListComponent->addListener(this);
    addAndMakeVisible(*fListComponent);
  }
}

ChooseOutputComponent::~ChooseOutputComponent() {
  fListComponent.reset();
  fList.reset();
}

void ChooseOutputComponent::paint(juce::Graphics &g) {
  /* This demo code just fills the component's background and
     draws some placeholder text to get you started.

     You should replace everything in this method with your own
     drawing code..
  */

  g.fillAll(getLookAndFeel().findColour(
      juce::ResizableWindow::backgroundColourId)); // clear the background

  g.setColour(juce::Colours::grey);
  g.drawRect(getLocalBounds(), 1); // draw an outline around the component

  g.setColour(juce::Colours::white);
  g.setFont(14.0f);
  g.drawText("ChooseOutputComponent", getLocalBounds(),
             juce::Justification::centred, true); // draw some placeholder text
}

void ChooseOutputComponent::resized() {
  // This method is where you should set the bounds of any child
  // components that your component contains..
}

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
  // TODO: alert overwriting existing directory
  JUCEApplication::getInstance()->perform({gui::toCopy});
}

void ChooseOutputComponent::browserRootChanged(const File &newRoot) {}

void ChooseOutputComponent::onSaveButtonClicked() {
  if (fState.fCopyDestinationDirectory) {
    JUCEApplication::getInstance()->perform({gui::toCopy});
  }
}