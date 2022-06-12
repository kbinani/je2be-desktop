#include <je2be.hpp>

#include "AsyncHandler.h"
#include "CommandID.h"
#include "Constants.h"
#include "File.h"
#include "TaskbarProgress.h"
#include "TemporaryDirectory.h"
#include "component/TextButton.h"
#include "component/j2b/J2BConvertProgress.h"

using namespace juce;

namespace je2be::desktop::component::j2b {

class J2BWorkerThread : public Thread, public je2be::tobe::Progress {
public:
  J2BWorkerThread(File input, File output, je2be::tobe::Options opt,
                  std::shared_ptr<AsyncHandler<J2BConvertProgress::UpdateQueue>> updater)
      : Thread("je2be::desktop::component::j2b::J2BWorkerThread"), fInput(input), fOutput(output), fOptions(opt), fUpdater(updater) {}

  void run() override {
    using UpdateQueue = J2BConvertProgress::UpdateQueue;

    je2be::tobe::Converter c(PathFromFile(fInput), PathFromFile(fOutput), fOptions);
    try {
      auto status = c.run(std::thread::hardware_concurrency(), this);
      triggerProgress(J2BConvertProgress::Phase::Done, 1, 1, status);
    } catch (std::filesystem::filesystem_error &e) {
      triggerError(Error(__FILE__, __LINE__, e.what()));
    } catch (std::exception &e) {
      triggerError(Error(__FILE__, __LINE__, e.what()));
    } catch (char const *what) {
      triggerError(Error(__FILE__, __LINE__, what));
    } catch (...) {
      triggerError(Error(__FILE__, __LINE__));
    }
  }

  bool report(je2be::tobe::Progress::Phase phase, double done, double total) override {
    J2BConvertProgress::Phase p;
    switch (phase) {
    case je2be::tobe::Progress::Phase::Convert:
      p = J2BConvertProgress::Phase::Convert;
      break;
    case je2be::tobe::Progress::Phase::LevelDbCompaction:
      p = J2BConvertProgress::Phase::LevelDBCompaction;
      break;
    default:
      p = J2BConvertProgress::Phase::Error;
      break;
    }
    triggerProgress(p, done, total);
    return !threadShouldExit();
  }

  void triggerProgress(J2BConvertProgress::Phase phase, double done, double total, Status st = Status::Ok()) {
    J2BConvertProgress::UpdateQueue q;
    q.fStatus = st;
    q.fPhase = phase;
    q.fTotal = total;
    q.fDone = done;
    fUpdater->trigger(q);
  }

  void triggerError(Status st) {
    J2BConvertProgress::UpdateQueue q;
    q.fStatus = st;
    q.fPhase = J2BConvertProgress::Phase::Error;
    q.fTotal = 1;
    q.fDone = 1;
    fUpdater->trigger(q);
  }

private:
  File const fInput;
  File const fOutput;
  je2be::tobe::Options const fOptions;
  std::shared_ptr<AsyncHandler<J2BConvertProgress::UpdateQueue>> fUpdater;
};

J2BConvertProgress::J2BConvertProgress(J2BConfigState const &configState) : fConfigState(configState) {
  auto width = kWindowWidth;
  auto height = kWindowHeight;
  setSize(width, height);

  fCancelButton.reset(new component::TextButton(TRANS("Cancel")));
  fCancelButton->setBounds(kMargin, height - kMargin - kButtonBaseHeight, kButtonMinWidth, kButtonBaseHeight);
  fCancelButton->onClick = [this]() { onCancelButtonClicked(); };
  addAndMakeVisible(*fCancelButton);

  int y = kMargin;
  fLabel.reset(new Label("", TRANS("Converting...")));
  fLabel->setBounds(kMargin, y, width - 2 * kMargin, kButtonBaseHeight);
  fLabel->setJustificationType(Justification::topLeft);
  addAndMakeVisible(*fLabel);
  int errorMessageY = y + fLabel->getHeight();
  y += fLabel->getHeight() + kMargin;

  fConversionProgressBar.reset(new ProgressBar(fConversionProgress));
  fConversionProgressBar->setBounds(kMargin, y, width - 2 * kMargin, kButtonBaseHeight);
  fConversionProgressBar->setTextToDisplay("Conversion: ");
  addAndMakeVisible(*fConversionProgressBar);
  y += fConversionProgressBar->getHeight() + kMargin;

  fCompactionProgressBar.reset(new ProgressBar(fCompactionProgress));
  fCompactionProgressBar->setBounds(kMargin, y, width - 2 * kMargin, kButtonBaseHeight);
  fCompactionProgressBar->setTextToDisplay("LevelDB Compaction: ");
  addAndMakeVisible(*fCompactionProgressBar);

  fErrorMessage.reset(new TextEditor());
  fErrorMessage->setBounds(kMargin, errorMessageY, width - 2 * kMargin, fCancelButton->getY() - kMargin - errorMessageY);
  fErrorMessage->setEnabled(false);
  fErrorMessage->setMultiLine(true);
  fErrorMessage->setColour(TextEditor::backgroundColourId, findColour(Label::backgroundColourId));
  addChildComponent(*fErrorMessage);

  fTaskbarProgress.reset(new TaskbarProgress());

  File temp = TemporaryDirectory::EnsureExisting();
  juce::Uuid u;
  File outputDir = temp.getChildFile(u.toDashedString());
  outputDir.createDirectory();
  fOutputDirectory = outputDir;

  fUpdater = std::make_shared<AsyncHandler<UpdateQueue>>([this](UpdateQueue q) {
    onProgressUpdate(q.fPhase, q.fDone, q.fTotal, q.fStatus);
  });

  je2be::tobe::Options opt;
  opt.fTempDirectory = PathFromFile(temp);
  if (fConfigState.fStructure == J2BConfigState::DirectoryStructure::Paper) {
    opt.fLevelDirectoryStructure = je2be::LevelDirectoryStructure::Paper;
  }
  fThread.reset(new J2BWorkerThread(configState.fInputState.fInput, outputDir, opt, fUpdater));
  fThread->startThread();
}

J2BConvertProgress::~J2BConvertProgress() {
  fThread->stopThread(-1);
  fTaskbarProgress->setState(TaskbarProgress::State::NoProgress);
}

void J2BConvertProgress::paint(juce::Graphics &g) {}

void J2BConvertProgress::onCancelButtonClicked() {
  if (fFailed) {
    JUCEApplication::getInstance()->invoke(commands::toChooseJavaInput, true);
  } else {
    fCancelButton->setEnabled(false);
    fCommandWhenFinished = commands::toJ2BConfig;
    fThread->signalThreadShouldExit();
    fConversionProgress = -1;
    fLabel->setText(TRANS("Waiting for the worker thread to finish"), dontSendNotification);
  }
}

void J2BConvertProgress::onProgressUpdate(J2BConvertProgress::Phase phase, double done, double total, Status st) {
  double weightConversion = 0.67;
  double weightCompaction = 1 - weightConversion;
  fFailed = !st.ok();

  if (phase == J2BConvertProgress::Phase::Done) {
    if (fCommandWhenFinished != commands::toChooseBedrockOutput && fOutputDirectory.exists()) {
      TemporaryDirectory::QueueDeletingDirectory(fOutputDirectory);
    }
    if (st.ok()) {
      fState = BedrockConvertedState(fConfigState.fInputState.fWorldName, fOutputDirectory);
      JUCEApplication::getInstance()->invoke(fCommandWhenFinished, true);
    }
    if (fFailed) {
      fTaskbarProgress->setState(TaskbarProgress::State::Error);
    } else {
      fTaskbarProgress->setState(TaskbarProgress::State::NoProgress);
    }
  } else if (phase == J2BConvertProgress::Phase::LevelDBCompaction) {
    fLabel->setText(TRANS("LevelDB compaction"), dontSendNotification);
    double progress = done / total;
    fConversionProgress = 1;
    fCompactionProgress = progress;
    fTaskbarProgress->setState(TaskbarProgress::State::Normal);
    fTaskbarProgress->update(weightConversion + progress * weightCompaction);
  } else if (phase == J2BConvertProgress::Phase::Convert) {
    if (fConversionProgress >= 0) {
      double progress = done / total;
      fConversionProgress = progress;
      fCompactionProgress = 0;
      fTaskbarProgress->setState(TaskbarProgress::State::Normal);
      fTaskbarProgress->update(progress * weightConversion);
    }
  } else {
    fFailed = true;
  }
  if (fFailed) {
    fLabel->setText(TRANS("The conversion failed."), dontSendNotification);
    fLabel->setColour(Label::textColourId, kErrorTextColor);
    auto error = st.error();
    if (error) {
      juce::String message = juce::String(JUCE_APPLICATION_NAME_STRING) + " version " + JUCE_APPLICATION_VERSION_STRING;
      message += juce::String("\nFailed: where: ") + error->fWhere.fFile + ":" + std::to_string(error->fWhere.fLine);
      if (!error->fWhat.empty()) {
        message += juce::String(", what: " + error->fWhat);
      }
      fErrorMessage->setText(message);
      fErrorMessage->setVisible(true);
    }
    fCancelButton->setButtonText(TRANS("Back"));
    fConversionProgressBar->setVisible(false);
    fCompactionProgressBar->setVisible(false);
  }
}

} // namespace je2be::desktop::component::j2b
