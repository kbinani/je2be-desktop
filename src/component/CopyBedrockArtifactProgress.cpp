#include "component/CopyBedrockArtifactProgress.h"

#include <je2be.hpp>

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
    u64 totalBytes = 0;
    for (auto const &item : RangedDirectoryIterator(fFrom, true)) {
      if (item.isDirectory()) {
        continue;
      }
      totalBytes += item.getFileSize();
    }

    je2be::ZipFile zip(PathFromFile(fTo));
    u64 writtenBytes = 0;

    bool cancelled = false;
    for (auto const &item : RangedDirectoryIterator(fFrom, true)) {
      if (item.isDirectory()) {
        continue;
      }
      i64 size = item.getFileSize();
      File file = item.getFile();
      juce::String relativePath = file.getRelativePathFrom(fFrom);
      int compressionLevel = 9;
      if (relativePath.startsWith("db" + File::getSeparatorString()) && relativePath.endsWith(".ldb")) {
        // Already compressed so just store it
        compressionLevel = 0;
      }
      auto in = std::make_shared<mcfile::stream::FileInputStream>(PathFromFile(file));
      if (!in->valid()) {
        fResult = CopyBedrockArtifactProgress::Worker::Result::Failed(Error(__FILE__, __LINE__));
        return;
      }
      auto st = zip.store(*in, std::string(relativePath.toUTF8()), compressionLevel);
      if (!st.fStatus.ok()) {
        fResult = CopyBedrockArtifactProgress::Worker::Result::Failed(st.fStatus);
        return;
      }
      if (st.fZip64Used) {
        fResult = CopyBedrockArtifactProgress::Worker::Result::TooLargeOutput();
        return;
      }
      if (currentThreadShouldExit()) {
        cancelled = true;
        break;
      }
      writtenBytes += size;
      *fProgress = writtenBytes / (double)totalBytes;
    }

    if (cancelled || currentThreadShouldExit()) {
      fResult = CopyBedrockArtifactProgress::Worker::Result::Cancelled();
      return;
    }
    auto result = zip.close();
    if (result.fZip64Used) {
      fResult = CopyBedrockArtifactProgress::Worker::Result::TooLargeOutput();
      return;
    }
    if (!result.fStatus.ok()) {
      fResult = CopyBedrockArtifactProgress::Worker::Result::Failed(result.fStatus);
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
    auto options = MessageBoxOptions()                                        //
                       .withIconType(AlertWindow::AlertIconType::WarningIcon) //
                       .withTitle(TRANS("Failed"))                            //
                       .withMessage(message)                                  //
                       .withButton("OK");
    AlertWindow::showAsync(options, [](int) { JUCEApplication::getInstance()->invoke(commands::toChooseBedrockOutput, true); });
  } else if (result->fType == CopyBedrockArtifactProgress::Worker::Result::Type::Cancelled) {
    fTaskbarProgress->setState(TaskbarProgress::State::NoProgress);
    auto options = MessageBoxOptions()                                     //
                       .withIconType(AlertWindow::AlertIconType::InfoIcon) //
                       .withTitle(TRANS("Cancelled"))                      //
                       .withMessage(TRANS("Saving cancelled."))            //
                       .withButton("OK");
    AlertWindow::showAsync(options, [](int) { JUCEApplication::getInstance()->invoke(commands::toChooseBedrockOutput, true); });
  } else if (result->fType == CopyBedrockArtifactProgress::Worker::Result::Type::TooLargeOutput) {
    juce::String message = TRANS("The size of the mcworld file has exceeded 4 GB.\rSuch mcworld files will result in loading errors,\rso please choose another export method.");
    auto options = MessageBoxOptions()                                        //
                       .withIconType(AlertWindow::AlertIconType::WarningIcon) //
                       .withTitle(TRANS("Failed"))                            //
                       .withMessage(message)                                  //
                       .withButton("OK");
    fState.fConvertedState.fDisableMCWorldExport = true;
    fState.fFormat = BedrockOutputFormat::Directory;
    AlertWindow::showAsync(options, [](int) { JUCEApplication::getInstance()->invoke(commands::toChooseBedrockOutput, true); });
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

void CopyBedrockArtifactProgress::timerCallback() {
  double progress = fProgress;
  fTaskbarProgress->update(progress);
}

} // namespace je2be::desktop::component
