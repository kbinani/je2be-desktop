#include "ChooseInputComponent.h"
#include "CommandID.h"
#include "Constants.h"
#include <JuceHeader.h>

ChooseInputComponent::ChooseInputComponent()
    : fListThread("j2b::gui::ChooseInputComponent") {
  auto width = kWindowWidth;
  auto height = kWindowHeight;
  auto fileListWidth = 280;

  setSize(width, height);
  {
    fMessage.reset(
        new Label("", TRANS("Select the world you want to convert")));
    fMessage->setBounds(
        kMargin, kMargin, width - kMargin - fileListWidth - kMargin - kMargin,
        height - kMargin - kButtonBaseHeight - kMargin - kMargin);
    fMessage->setJustificationType(Justification::topLeft);
    fMessage->setMinimumHorizontalScale(1);
    addAndMakeVisible(*fMessage);
  }
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
    fListComponent.reset(new FileListComponent(*fList));
    fListComponent->setBounds(width - kMargin - fileListWidth, kMargin,
                              fileListWidth,
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
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void ChooseInputComponent::onNextButtonClicked() {
  JUCEApplication::getInstance()->perform({gui::toConfig});
}

void ChooseInputComponent::onChooseCustomButtonClicked() {
  // TODO:
}

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
