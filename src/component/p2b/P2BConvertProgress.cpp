#include <je2be.hpp>

#include "CommandID.h"
#include "Constants.h"
#include "File.h"
#include "TemporaryDirectory.h"
#include "component/p2b/P2BConvertProgress.h"

using namespace juce;

namespace je2be::desktop::component::p2b {

class P2BWorkerThread : public Thread, public je2be::lce::Progress, public je2be::java::Progress {
public:
  P2BWorkerThread(File input, File output, std::weak_ptr<ConvertProgress> updater, File tempRoot)
      : Thread("je2be::desktop::component::p2b::P2BWorkerThread"), fInput(input), fOutput(output), fUpdater(updater), fTempRoot(tempRoot) {}

  void run() override {
    try {
      unsafeRun();
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

  void unsafeRun() {
    juce::Uuid u;
    File javaIntermediateDirectory = fTempRoot.getChildFile(u.toDashedString());
    if (auto result = javaIntermediateDirectory.createDirectory(); !result.ok()) {
      triggerError(Error(__FILE__, __LINE__, result.getErrorMessage().toStdString()));
      return;
    }
    {
      je2be::lce::Options o;
      o.fTempDirectory = PathFromFile(fTempRoot);
      auto status = je2be::ps3::Converter::Run(PathFromFile(fInput), PathFromFile(javaIntermediateDirectory), std::thread::hardware_concurrency(), o, this);
      auto updater = fUpdater.lock();
      if (!updater) {
        return;
      }
      if (!status.ok()) {
        updater->notifyError(status);
        return;
      }
    }
    if (threadShouldExit()) {
      triggerError(Status::Ok());
      return;
    }
    {
      je2be::java::Options o;
      o.fTempDirectory = PathFromFile(fTempRoot);
      auto status = je2be::java::Converter::Run(PathFromFile(javaIntermediateDirectory), PathFromFile(fOutput), o, std::thread::hardware_concurrency(), this);
      auto updater = fUpdater.lock();
      if (!updater) {
        return;
      }
      if (status.ok()) {
        updater->notifyFinished();
      } else {
        updater->notifyError(status);
      }
    }
  }

  bool report(Rational<u64> const &progress) override {
    if (auto updater = fUpdater.lock(); updater) {
      ConvertProgress::Progress p;
      p.fProgress = progress.toD();
      p.fCount = 0;
      updater->notifyProgress(0, p);
    }
    return !threadShouldExit();
  }

  bool reportConvert(Rational<u64> const &progress, uint64_t numConvertedChunks) override {
    if (auto updater = fUpdater.lock(); updater) {
      ConvertProgress::Progress p;
      p.fProgress = progress.toD();
      p.fCount = numConvertedChunks;
      updater->notifyProgress(1, p);
    }
    return !threadShouldExit();
  }

  bool reportEntityPostProcess(Rational<u64> const &progress) override {
    if (auto updater = fUpdater.lock(); updater) {
      ConvertProgress::Progress p;
      p.fProgress = progress.toD();
      p.fCount = 0;
      updater->notifyProgress(2, p);
    }
    return !threadShouldExit();
  }

  bool reportCompaction(Rational<u64> const &progress) override {
    if (auto updater = fUpdater.lock(); updater) {
      ConvertProgress::Progress p;
      p.fProgress = progress.toD();
      p.fCount = 0;
      updater->notifyProgress(3, p);
    }
    return !threadShouldExit();
  }

  void triggerError(Status st) {
    if (auto updater = fUpdater.lock(); updater) {
      updater->notifyError(st);
    }
  }

private:
  File const fInput;
  File const fOutput;
  File const fTempRoot;
  std::weak_ptr<ConvertProgress> fUpdater;
};

P2BConvertProgress::P2BConvertProgress(ToBedrockConfigState const &configState) : fConfigState(configState) {
  setSize(kWindowWidth, kWindowHeight);

  fTempRoot = TemporaryDirectory::EnsureExisting();
  juce::Uuid u;
  File outputDir = fTempRoot.getChildFile(u.toDashedString());
  outputDir.createDirectory();
  fOutputDirectory = outputDir;
}

void P2BConvertProgress::startThread() {
  fThread.reset(new P2BWorkerThread(fConfigState.fInputState.fInput, fOutputDirectory, weak_from_this(), fTempRoot));
  fThread->startThread();
}

void P2BConvertProgress::onCancelButtonClicked() {
  if (fFailure) {
    JUCEApplication::getInstance()->invoke(commands::toChoosePS3InputToBedrock, true);
  } else {
    fCommandWhenFinished = commands::toPS3ToBedrockConfig;
  }
}

void P2BConvertProgress::onFinish() {
  if (fCommandWhenFinished != commands::toChooseBedrockOutput && fOutputDirectory.exists()) {
    TemporaryDirectory::QueueDeletingDirectory(fOutputDirectory);
  }
  if (!fFailure) {
    fState = BedrockConvertedState(fConfigState.fInputState.fWorldName, fOutputDirectory);
    JUCEApplication::getInstance()->invoke(fCommandWhenFinished, true);
  }
}

} // namespace je2be::desktop::component::p2b
