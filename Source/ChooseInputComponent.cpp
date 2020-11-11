#include "ChooseInputComponent.h"
#include "CommandID.h"
#include "Constants.h"
#include <JuceHeader.h>

ChooseInputComponent::ChooseInputComponent()
    : fListThread("j2b::gui::ChooseInputComponent") {
  auto width = kWindowWidth;
  auto height = kWindowHeight;
  setSize(width, height);
  {
    fNextButton.reset(new TextButton(TRANS("Next")));
    fNextButton->setBounds(width - kButtonMinWidth - kMargin,
                           height - kButtonBaseHeight - kMargin,
                           kButtonMinWidth, kButtonBaseHeight);
    fNextButton->onClick = [this]() { onNextButtonClicked(); };
    fNextButton->setEnabled(false);
    addAndMakeVisible(*fNextButton);
  }

  {
    auto w = 140;
    fChooseCustomButton.reset(new TextButton(TRANS("Custom Folder")));
    fChooseCustomButton->setBounds(
        width - kButtonMinWidth - kMargin - w - kMargin,
        height - kButtonBaseHeight - kMargin, w, kButtonBaseHeight);
    fChooseCustomButton->onClick = [this]() { onChooseCustomButtonClicked(); };
    addAndMakeVisible(*fChooseCustomButton);
  }

  fListThread.startThread();
  fList.reset(new DirectoryContentsList(nullptr, fListThread));

  File dir = File::getSpecialLocation(File::userApplicationDataDirectory)
                 .getChildFile(".minecraft")
                 .getChildFile("saves");
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

ChooseInputComponent::~ChooseInputComponent() {
  fListComponent.reset();
  fList.reset();
}

void ChooseInputComponent::paint(juce::Graphics &g) {
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
  g.drawText("ChooseInputComponent", getLocalBounds(),
             juce::Justification::centred, true); // draw some placeholder text
}

void ChooseInputComponent::resized() {}

void ChooseInputComponent::onNextButtonClicked() {
  JUCEApplication::getInstance()->perform({gui::toConfig});
}

void ChooseInputComponent::onChooseCustomButtonClicked() {}

void ChooseInputComponent::selectionChanged() {
  int num = fListComponent->getNumSelectedFiles();
  if (num == 1) {
    fState.fInputDirectory = fListComponent->getSelectedFile();
  } else {
    fState.fInputDirectory = std::nullopt;
  }
  fNextButton->setEnabled(fState.fInputDirectory != std::nullopt);
}

void ChooseInputComponent::fileClicked(const File &file, const MouseEvent &e) {}

void ChooseInputComponent::fileDoubleClicked(const File &file) {
  fState.fInputDirectory = file;
  JUCEApplication::getInstance()->perform({gui::toConfig});
}

void ChooseInputComponent::browserRootChanged(const File &newRoot) {}
