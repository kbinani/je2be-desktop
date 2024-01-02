#include <je2be.hpp>

#include "component/j2b/J2BConvertProgress.h"

#include "CommandID.h"
#include "Constants.h"
#include "File.h"
#include "TemporaryDirectory.h"

using namespace juce;

namespace je2be::desktop::component::j2b {

class J2BWorkerThread : public Thread, public je2be::java::Progress {
public:
  J2BWorkerThread(File input, File output, je2be::java::Options opt, std::weak_ptr<ConvertProgress> updater)
      : Thread("je2be::desktop::component::j2b::J2BWorkerThread"), fInput(input), fOutput(output), fOptions(opt), fUpdater(updater) {}

  void run() override {
    try {
      auto status = je2be::java::Converter::Run(PathFromFile(fInput), PathFromFile(fOutput), fOptions, std::thread::hardware_concurrency(), this);
      auto updater = fUpdater.lock();
      if (!updater) {
        return;
      }
      if (status.ok()) {
        updater->notifyFinished();
      } else {
        updater->notifyError(status);
      }
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

  bool reportConvert(Rational<u64> const &progress, uint64_t numConvertedChunks) override {
    if (auto updater = fUpdater.lock(); updater) {
      ConvertProgress::Progress p;
      p.fProgress = progress.toD();
      p.fCount = numConvertedChunks;

      updater->notifyProgress(0, p);
    }
    return !threadShouldExit();
  }

  bool reportEntityPostProcess(Rational<u64> const &progress) override {
    if (auto updater = fUpdater.lock(); updater) {
      ConvertProgress::Progress p;
      p.fProgress = progress.toD();
      p.fCount = 0;

      updater->notifyProgress(1, p);
    }
    return !threadShouldExit();
  }

  bool reportCompaction(Rational<u64> const &progress) override {
    if (auto updater = fUpdater.lock(); updater) {
      ConvertProgress::Progress p;
      p.fProgress = progress.toD();
      p.fCount = 0;

      updater->notifyProgress(2, p);
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
  je2be::java::Options const fOptions;
  std::weak_ptr<ConvertProgress> fUpdater;
  Status fStatus = Status::Ok();
};

J2BConvertProgress::J2BConvertProgress(J2BConfigState const &configState) : fConfigState(configState) {
  setSize(kWindowWidth, kWindowHeight);

  fTempRoot = TemporaryDirectory::EnsureExisting();
  juce::Uuid u;
  File outputDir = fTempRoot.getChildFile(u.toDashedString());
  outputDir.createDirectory();
  fOutputDirectory = outputDir;
}

void J2BConvertProgress::startThread() {
  je2be::java::Options opt;
  opt.fTempDirectory = PathFromFile(fTempRoot);
  if (fConfigState.fStructure == J2BConfigState::DirectoryStructure::Paper) {
    opt.fLevelDirectoryStructure = je2be::LevelDirectoryStructure::Paper;
  }
  fThread.reset(new J2BWorkerThread(fConfigState.fInputState.fInput, fOutputDirectory, opt, weak_from_this()));
  fThread->startThread();
}

void J2BConvertProgress::onCancelButtonClicked() {
  if (fFailure) {
    JUCEApplication::getInstance()->invoke(commands::toChooseJavaInput, true);
  } else {
    fCommandWhenFinished = commands::toJ2BConfig;
  }
}

void J2BConvertProgress::onFinish() {
  if (fCommandWhenFinished != commands::toChooseBedrockOutput && fOutputDirectory.exists()) {
    TemporaryDirectory::QueueDeletingDirectory(fOutputDirectory);
  }
  if (!fFailure) {
    fState = BedrockConvertedState(fConfigState.fInputState.fWorldName, fOutputDirectory);
    JUCEApplication::getInstance()->invoke(fCommandWhenFinished, true);
  }
}

} // namespace je2be::desktop::component::j2b
