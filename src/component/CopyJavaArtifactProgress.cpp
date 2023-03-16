#include "component/CopyJavaArtifactProgress.h"
#include "CommandID.h"
#include "ComponentState.h"
#include "Constants.h"
#include "File.h"
#include "TaskbarProgress.h"
#include "TemporaryDirectory.h"

using namespace juce;

namespace je2be::desktop::component {

class CopyJavaArtifactThread : public CopyJavaArtifactProgress::Worker {
public:
  CopyJavaArtifactThread(AsyncUpdater *updater, File from, File to, double *progress)
      : CopyJavaArtifactProgress::Worker("je2be::desktop::component::CopyJavaArtifactThread"),
        fUpdater(updater), fFrom(from), fTo(to), fProgress(progress) {}

  void run() override {
    try {
      unsafeRun();
    } catch (std::filesystem::filesystem_error &e) {
      fResult = CopyJavaArtifactProgress::Worker::Result::Failed(Error(__FILE__, __LINE__, e.what()));
    } catch (std::exception &e) {
      fResult = CopyJavaArtifactProgress::Worker::Result::Failed(Error(__FILE__, __LINE__, e.what()));
    } catch (char const *what) {
      fResult = CopyJavaArtifactProgress::Worker::Result::Failed(Error(__FILE__, __LINE__, what));
    } catch (...) {
      fResult = CopyJavaArtifactProgress::Worker::Result::Failed(Error(__FILE__, __LINE__));
    }
    fUpdater->triggerAsyncUpdate();
  }

  std::optional<CopyJavaArtifactProgress::Worker::Result> result() const override {
    return fResult;
  }

private:
  void unsafeRun() {
    RangedDirectoryIterator it(fFrom, true);
    for (auto const &item : it) {
      File const &from = item.getFile();
      if (from.isDirectory()) {
        continue;
      }
      auto relative = from.getRelativePathFrom(fFrom);
      auto destination = fTo.getChildFile(relative);
      auto dir = destination.getParentDirectory();
      if (!dir.exists()) {
        auto result = dir.createDirectory();
        if (!result.wasOk()) {
          fResult = CopyJavaArtifactProgress::Worker::Result::Failed(Error(__FILE__, __LINE__, result.getErrorMessage().toStdString()));
          return;
        }
      }
      if (auto st = CopyFile(from, destination, __FILE__, __LINE__); !st.ok()) {
        fResult = CopyJavaArtifactProgress::Worker::Result::Failed(st);
        return;
      }
      *fProgress = (double)item.getEstimatedProgress();
    }
    fResult = CopyJavaArtifactProgress::Worker::Result::Success();
  }

private:
  AsyncUpdater *const fUpdater;
  File fFrom;
  File fTo;
  std::optional<CopyJavaArtifactProgress::Worker::Result> fResult;
  double *const fProgress;
};

CopyJavaArtifactProgress::CopyJavaArtifactProgress(JavaOutputChoosenState const &state) : fState(state) {
  auto width = kWindowWidth;
  auto height = kWindowHeight;
  setSize(width, height);

  fLabel.reset(new Label("", TRANS("Saving...")));
  fLabel->setBounds(kMargin, kMargin, width - 2 * kMargin, kButtonBaseHeight);
  fLabel->setJustificationType(Justification::topLeft);
  addAndMakeVisible(*fLabel);

  fProgressBar.reset(new ProgressBar(fProgress));
  fProgressBar->setBounds(kMargin, kMargin + kButtonBaseHeight + kMargin, width - 2 * kMargin, kButtonBaseHeight);
  addAndMakeVisible(*fProgressBar);

  fTaskbarProgress.reset(new TaskbarProgress());

  fCopyThread.reset(new CopyJavaArtifactThread(this, state.fConvertedState.fOutputDirectory, *state.fCopyDestination, &fProgress));
  fCopyThread->startThread();
  fTaskbarProgress->setState(TaskbarProgress::State::Normal);
  startTimerHz(12);
}

CopyJavaArtifactProgress::~CopyJavaArtifactProgress() {
  fTaskbarProgress->setState(TaskbarProgress::State::NoProgress);
}

void CopyJavaArtifactProgress::paint(juce::Graphics &g) {}

void CopyJavaArtifactProgress::handleAsyncUpdate() {
  stopTimer();

  auto result = fCopyThread->result();
  if (!result || result->fType == CopyJavaArtifactProgress::Worker::Result::Type::Failed) {
    fTaskbarProgress->setState(TaskbarProgress::State::Error);
    juce::String message = TRANS("Saving failed.");
    if (result && result->fStatus.error()) {
      auto error = result->fStatus.error();
      if (!error->fWhat.empty()) {
        message += juce::String("  what: " + error->fWhat + "\n");
      }
      message += juce::String("  trace: \n");
      for (int i = error->fTrace.size() - 1; i >= 0; i--) {
        auto const &trace = error->fTrace[i];
        message += juce::String("    " + trace.fFile + ":" + std::to_string(trace.fLine) + "\n");
      }
      message = message.trimEnd();
    }
    auto options = MessageBoxOptions()                                        //
                       .withIconType(AlertWindow::AlertIconType::WarningIcon) //
                       .withTitle(TRANS("Failed"))                            //
                       .withMessage(message)                                  //
                       .withButton("OK");
    AlertWindow::showAsync(options, [](int) { JUCEApplication::getInstance()->invoke(commands::toChooseJavaOutput, true); });
  } else if (result->fType == CopyJavaArtifactProgress::Worker::Result::Type::Cancelled) {
    auto options = MessageBoxOptions()                                     //
                       .withIconType(AlertWindow::AlertIconType::InfoIcon) //
                       .withTitle(TRANS("Cancelled"))                      //
                       .withMessage(TRANS("Saving cancelled."))            //
                       .withButton("OK");
    AlertWindow::showAsync(options, [](int) { JUCEApplication::getInstance()->invoke(commands::toChooseJavaOutput, true); });
  } else {
    fTaskbarProgress->setState(TaskbarProgress::State::NoProgress);
    auto options = MessageBoxOptions()                                                                              //
                       .withIconType(AlertWindow::AlertIconType::InfoIcon)                                          //
                       .withTitle(TRANS("Completed"))                                                               //
                       .withMessage(TRANS("Saving completed.") + "\n" + fState.fCopyDestination->getFullPathName()) //
                       .withButton("OK")                                                                            //
                       .withButton(TRANS("Show in Explorer"));
    File copyDestination = *fState.fCopyDestination;
    AlertWindow::showAsync(options, [copyDestination](int returnValue) {
      if (returnValue == 0) {
        copyDestination.revealToUser();
      }
      JUCEApplication::getInstance()->invoke(commands::toModeSelect, true);
    });
    if (fState.fConvertedState.fOutputDirectory.exists()) {
      TemporaryDirectory::QueueDeletingDirectory(fState.fConvertedState.fOutputDirectory);
    }
  }
}

void CopyJavaArtifactProgress::timerCallback() {
  double progress = fProgress;
  fTaskbarProgress->update(progress);
}

} // namespace je2be::desktop::component
