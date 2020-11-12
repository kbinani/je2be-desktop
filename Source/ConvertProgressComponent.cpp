#include "ConvertProgressComponent.h"
#include "CommandID.h"
#include "Constants.h"
#include <JuceHeader.h>
#include <je2be.hpp>

class WorkerThread : public Thread, public j2b::Progress {
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
    c.run(std::thread::hardware_concurrency(), this);
    fUpdater->trigger(2, 1, 1);
  }

  bool report(j2b::Progress::Phase phase, double done, double total) override {
    int p = 0;
    switch (phase) {
    case j2b::Progress::Phase::Convert:
      p = 0;
      break;
    case j2b::Progress::Phase::LevelDbCompaction:
      p = 1;
      break;
    }
    fUpdater->trigger(p, done, total);
    return !threadShouldExit();
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

  fLabel.reset(new Label("", TRANS("Converting...")));
  fLabel->setBounds(kMargin, kMargin, width - 2 * kMargin, kButtonBaseHeight);
  addAndMakeVisible(*fLabel);

  fProgressBar.reset(new ProgressBar(fProgress));
  fProgressBar->setBounds(kMargin, kMargin + kButtonBaseHeight + kMargin,
                          width - 2 * kMargin, kButtonBaseHeight);
  addAndMakeVisible(*fProgressBar);

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
  fThread->stopThread(-1);
}

void ConvertProgressComponent::paint(juce::Graphics &g) {
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void ConvertProgressComponent::onCancelButtonClicked() {
  fCancelButton->setEnabled(false);
  fCommandWhenFinished = gui::toConfig;
  fThread->signalThreadShouldExit();
  fProgress = -1;
  fLabel->setText(TRANS("Waiting for the worker thread to finish"),
                  dontSendNotification);
}

void ConvertProgressComponent::onProgressUpdate(int phase, double done,
                                                double total) {
  if (phase == 2) {
    if (fCommandWhenFinished != gui::toChooseOutput &&
        fState.fOutputDirectory.exists()) {
      fState.fOutputDirectory.deleteRecursively();
    }
    JUCEApplication::getInstance()->perform({fCommandWhenFinished});
  } else if (phase == 1) {
    fLabel->setText(TRANS("LevelDB compaction"), dontSendNotification);
    fProgress = -1;
  } else {
    if (fProgress >= 0) {
      fProgress = done / total;
    }
  }
}
