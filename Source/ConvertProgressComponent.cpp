#include "ConvertProgressComponent.h"
#include "CommandID.h"
#include "Constants.h"
#include <JuceHeader.h>
#include <je2be.hpp>

class WorkerThread : public Thread {
public:
  WorkerThread(File input, j2b::InputOption io, File output,
               j2b::OutputOption oo,
               std::shared_ptr<ConvertProgressComponent::Updater> updater)
      : Thread("j2b::gui::Convert"), fInput(input), fInputOption(io),
        fOutput(output), fOutputOption(oo), fUpdater(updater) {}

  void run() override {
    using namespace j2b;
    Converter c(fInput.getFullPathName().toStdString(), fInputOption,
                fOutput.getFullPathName().toStdString(), fOutputOption);
    c.run(std::thread::hardware_concurrency());
    fUpdater->triggerAsyncUpdate();
  }

private:
  File const fInput;
  j2b::InputOption const fInputOption;
  File const fOutput;
  j2b::OutputOption const fOutputOption;
  std::shared_ptr<ConvertProgressComponent::Updater> fUpdater;
};

ConvertProgressComponent::ConvertProgressComponent(
    ConfigState const &configState)
    : fState(configState) {
  auto width = kWindowWidth;
  auto height = kWindowHeight;
  setSize(width, height);

  fCancelButton.reset(new TextButton(TRANS("Cancel")));
  fCancelButton->setBounds(kMargin, height - kMargin - kButtonBaseHeight,
                           kButtonMinWidth, kButtonBaseHeight);
  fCancelButton->onClick = [this]() { onCancelButtonClicked(); };
  addAndMakeVisible(*fCancelButton);

  wchar_t buffer[L_tmpnam_s];
  _wtmpnam_s(buffer);
  fState.fOutputDirectory = File(String(buffer, L_tmpnam_s));

  fUpdater = std::make_shared<Updater>();
  fUpdater->fTarget.store(this);

  fThread.reset(new WorkerThread(*configState.fInputState.fInputDirectory, {},
                                 fState.fOutputDirectory, {}, fUpdater));
  fThread->startThread();
}

ConvertProgressComponent::~ConvertProgressComponent() {
  fThread->stopThread(1000);
  fUpdater->fTarget.store(nullptr);
}

void ConvertProgressComponent::paint(juce::Graphics &g) {
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
  g.drawText("ConvertProgressComponent", getLocalBounds(),
             juce::Justification::centred, true); // draw some placeholder text
}

void ConvertProgressComponent::resized() {}

void ConvertProgressComponent::onCancelButtonClicked() {
  JUCEApplication::getInstance()->perform({gui::toConfig});
}

void ConvertProgressComponent::onProgressUpdate() {
  if (fThread->isThreadRunning()) {
    // TODO: upate progress
    return;
  }
  JUCEApplication::getInstance()->perform({gui::toChooseOutput});
}