#include "component/CopyBedrockArtifactProgress.h"
#include "CommandID.h"
#include "ComponentState.h"
#include "Constants.h"
#include "File.h"
#include "TaskbarProgress.h"
#include "TemporaryDirectory.h"

using namespace juce;

namespace je2be::desktop::component {

class CopyBedrockArtifactThread : public CopyBedrockArtifactProgress::Worker {
public:
  CopyBedrockArtifactThread(AsyncUpdater *updater, File from, File to, double *progress)
      : CopyBedrockArtifactProgress::Worker("je2be::desktop::component::CopyBedrockArtifactThread"),
        fUpdater(updater), fFrom(from), fTo(to), fProgress(progress) {}

  void run() override {
    try {
      unsafeRun();
    } catch (std::filesystem::filesystem_error &e) {
      fResult = CopyBedrockArtifactProgress::Worker::Result::Failed(Error(__FILE__, __LINE__, e.what()));
    } catch (std::exception &e) {
      fResult = CopyBedrockArtifactProgress::Worker::Result::Failed(Error(__FILE__, __LINE__, e.what()));
    } catch (char const *what) {
      fResult = CopyBedrockArtifactProgress::Worker::Result::Failed(Error(__FILE__, __LINE__, what));
    } catch (...) {
      fResult = CopyBedrockArtifactProgress::Worker::Result::Failed(Error(__FILE__, __LINE__));
    }
    fUpdater->triggerAsyncUpdate();
  }

  std::optional<CopyBedrockArtifactProgress::Worker::Result> result() const override {
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
          fResult = CopyBedrockArtifactProgress::Worker::Result::Failed(Error(__FILE__, __LINE__, result.getErrorMessage().toStdString()));
          return;
        }
      }
      if (auto st = CopyFile(from, destination, __FILE__, __LINE__); !st.ok()) {
        fResult = CopyBedrockArtifactProgress::Worker::Result::Failed(st);
        return;
      }
      *fProgress = (double)item.getEstimatedProgress();
    }
    fResult = CopyBedrockArtifactProgress::Worker::Result::Success();
  }

private:
  AsyncUpdater *const fUpdater;
  File fFrom;
  File fTo;
  std::optional<CopyBedrockArtifactProgress::Worker::Result> fResult;
  double *const fProgress;
};

class ZipBedrockArtifactThread : public CopyBedrockArtifactProgress::Worker {
public:
  ZipBedrockArtifactThread(AsyncUpdater *updater, File from, File to, double *progress)
      : CopyBedrockArtifactProgress::Worker("je2be::desktop::component::ZipBedrockArtifactThread"), fUpdater(updater),
        fFrom(from), fTo(to), fProgress(progress) {}

  void run() override {
    try {
      unsafeRun();
    } catch (std::filesystem::filesystem_error &e) {
      fResult = CopyBedrockArtifactProgress::Worker::Result::Failed(Error(__FILE__, __LINE__, e.what()));
    } catch (std::exception &e) {
      fResult = CopyBedrockArtifactProgress::Worker::Result::Failed(Error(__FILE__, __LINE__, e.what()));
    } catch (char const *what) {
      fResult = CopyBedrockArtifactProgress::Worker::Result::Failed(Error(__FILE__, __LINE__, what));
    } catch (...) {
      fResult = CopyBedrockArtifactProgress::Worker::Result::Failed(Error(__FILE__, __LINE__));
    }
    fUpdater->triggerAsyncUpdate();
  }

  std::optional<CopyBedrockArtifactProgress::Worker::Result> result() const override {
    return fResult;
  }

private:
  void unsafeRun() {
    ZipFile::Builder builder;
    RangedDirectoryIterator it(fFrom, true);
    int const kCompressionLevel = 9;
    bool cancelled = false;
    for (auto const &item : it) {
      File file = item.getFile();
      String relativePath = file.getRelativePathFrom(fFrom);
      builder.addFile(file, kCompressionLevel, relativePath);
      if (currentThreadShouldExit()) {
        cancelled = true;
        break;
      }
    }

    if (cancelled || currentThreadShouldExit()) {
      fResult = CopyBedrockArtifactProgress::Worker::Result::Cancelled();
      return;
    }
    auto stream = fTo.createOutputStream();
    if (!stream) {
      fResult = CopyBedrockArtifactProgress::Worker::Result::Failed(Error(__FILE__, __LINE__));
      return;
    }
    if (!stream->setPosition(0)) {
      fResult = CopyBedrockArtifactProgress::Worker::Result::Failed(Error(__FILE__, __LINE__));
      return;
    }
    if (auto result = stream->truncate(); !result.ok()) {
      fResult = CopyBedrockArtifactProgress::Worker::Result::Failed(Error(__FILE__, __LINE__, result.getErrorMessage().toStdString()));
      return;
    }
    if (!builder.writeToStream(*stream, fProgress)) {
      fResult = CopyBedrockArtifactProgress::Worker::Result::Failed(Error(__FILE__, __LINE__));
      return;
    }
    fResult = CopyBedrockArtifactProgress::Worker::Result::Success();
  }

private:
  AsyncUpdater *const fUpdater;
  File fFrom;
  File fTo;
  std::optional<CopyBedrockArtifactProgress::Worker::Result> fResult;
  double *const fProgress;
};

CopyBedrockArtifactProgress::CopyBedrockArtifactProgress(BedrockOutputChoosenState const &state) : fState(state) {
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

  if (state.fFormat == BedrockOutputFormat::Directory) {
    fCopyThread.reset(new CopyBedrockArtifactThread(this, state.fConvertedState.fOutputDirectory, *state.fCopyDestination, &fProgress));
  } else {
    fCopyThread.reset(new ZipBedrockArtifactThread(this, state.fConvertedState.fOutputDirectory, *state.fCopyDestination, &fProgress));
  }
  fCopyThread->startThread();
  fTaskbarProgress->setState(TaskbarProgress::State::Normal);
  startTimerHz(12);
}

CopyBedrockArtifactProgress::~CopyBedrockArtifactProgress() {
  fTaskbarProgress->setState(TaskbarProgress::State::NoProgress);
}

void CopyBedrockArtifactProgress::paint(juce::Graphics &g) {}

void CopyBedrockArtifactProgress::handleAsyncUpdate() {
  struct InvokeToChooseOutput : public ModalComponentManager::Callback {
    void modalStateFinished(int returnValue) override {
      JUCEApplication::getInstance()->invoke(commands::toChooseBedrockOutput, true);
    }
  };

  struct InvokeToModeSelect : public ModalComponentManager::Callback {
    void modalStateFinished(int returnValue) override {
      JUCEApplication::getInstance()->invoke(commands::toModeSelect, true);
    }
  };

  stopTimer();

  auto result = fCopyThread->result();
  if (!result || result->fType == CopyBedrockArtifactProgress::Worker::Result::Type::Failed) {
    fTaskbarProgress->setState(TaskbarProgress::State::Error);
    juce::String message = TRANS("Saving failed.");
    if (result && result->fStatus.error()) {
      auto error = result->fStatus.error();
      message += juce::String(" where: " + error->fWhere.fFile + ":" + std::to_string(error->fWhere.fLine));
      if (!error->fWhat.empty()) {
        message += juce::String(", what: " + error->fWhat);
      }
    }
    AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon,
                                     TRANS("Failed"),
                                     message,
                                     "OK",
                                     nullptr,
                                     new InvokeToChooseOutput);
  } else if (result->fType == CopyBedrockArtifactProgress::Worker::Result::Type::Cancelled) {
    fTaskbarProgress->setState(TaskbarProgress::State::NoProgress);
    AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::InfoIcon,
                                     TRANS("Cancelled"),
                                     TRANS("Saving cancelled."),
                                     "OK",
                                     nullptr,
                                     new InvokeToChooseOutput);
    JUCEApplication::getInstance()->invoke(commands::toChooseBedrockOutput, true);
  } else {
    fTaskbarProgress->setState(TaskbarProgress::State::NoProgress);
    AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::InfoIcon,
                                     TRANS("Completed"),
                                     TRANS("Saving completed.") + "\n" + fState.fCopyDestination->getFullPathName(),
                                     "OK",
                                     nullptr,
                                     new InvokeToModeSelect);
    if (fState.fConvertedState.fOutputDirectory.exists()) {
      TemporaryDirectory::QueueDeletingDirectory(fState.fConvertedState.fOutputDirectory);
    }
  }
}

void CopyBedrockArtifactProgress::timerCallback() {
  double progress = fProgress;
  fTaskbarProgress->update(progress);
}

} // namespace je2be::desktop::component
