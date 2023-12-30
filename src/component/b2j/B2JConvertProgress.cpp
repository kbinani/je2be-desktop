#include <je2be.hpp>

#include <leveldb/env.h>

#include "CommandID.h"
#include "Constants.h"
#include "File.h"
#include "TaskbarProgress.h"
#include "TemporaryDirectory.h"
#include "component/TextButton.h"
#include "component/b2j/B2JConvertProgress.h"

using namespace juce;

namespace je2be::desktop::component::b2j {

class B2JWorkerThread : public Thread, public je2be::toje::Progress {
public:
  B2JWorkerThread(File input, File output, je2be::toje::Options opt, int stepOffset, std::weak_ptr<ConvertProgress> updater)
      : Thread("je2be::desktop::component::b2j::B2JWorkerThread"), fInput(input), fOutput(output), fOptions(opt), fUpdater(updater), fStepOffset(stepOffset) {}

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
    File temp;
    defer {
      if (temp != File()) {
        TemporaryDirectory::QueueDeletingDirectory(temp);
      }
    };

    File input;
    if (fInput.isDirectory()) {
      input = fInput;
    } else {
      File sessionTempDir = TemporaryDirectory::EnsureExisting();
      juce::Uuid u;
      temp = sessionTempDir.getChildFile(u.toDashedString());
      if (auto st = temp.createDirectory(); !st.ok()) {
        auto updater = fUpdater.lock();
        if (!updater) {
          return;
        }
        updater->notifyError(Error(__FILE__, __LINE__, st.getErrorMessage().toStdString()));
        return;
      }
      if (auto st = unzipInto(temp); !st.ok()) {
        triggerError(st);
        return;
      }
      input = temp;
    }
    auto st = je2be::toje::Converter::Run(PathFromFile(input), PathFromFile(fOutput), fOptions, std::thread::hardware_concurrency(), this);
    auto updater = fUpdater.lock();
    if (!updater) {
      return;
    }
    if (st.ok()) {
      updater->notifyFinished();
    } else {
      updater->notifyError(st);
    }
  }

  je2be::Status unzipInto(File temp) {
    juce::ZipFile file(fInput);
    int const total = file.getNumEntries();
    for (int i = 0; i < total; i++) {
      if (threadShouldExit()) {
        break;
      }
      auto ret = file.uncompressEntry(i, temp);
      if (!ret.ok()) {
        return Error(__FILE__, __LINE__, ret.getErrorMessage().toStdString());
      }
      if (auto updater = fUpdater.lock(); updater) {
        ConvertProgress::Progress p;
        p.fProgress = (i + 1) / (double)total;
        p.fCount = 0;
        updater->notifyProgress(0, p);
      } else {
        break;
      }
    }
    return je2be::Status::Ok();
  }

  bool reportConvert(Rational<u64> const &progress, uint64_t numConvertedChunks) override {
    if (auto updater = fUpdater.lock(); updater) {
      ConvertProgress::Progress p;
      p.fProgress = progress.toD();
      p.fCount = numConvertedChunks;
      updater->notifyProgress(fStepOffset, p);
    }
    return !threadShouldExit();
  }

  bool reportTerraform(Rational<u64> const &progress, uint64_t numProcessedChunks) override {
    if (auto updater = fUpdater.lock(); updater) {
      ConvertProgress::Progress p;
      p.fProgress = progress.toD();
      p.fCount = numProcessedChunks;
      updater->notifyProgress(fStepOffset + 1, p);
    }
    return !threadShouldExit();
  }

  void triggerError(Status st) {
    if (st.ok()) {
      return;
    }
    if (auto updater = fUpdater.lock(); updater) {
      updater->notifyError(st);
    }
  }

private:
  File const fInput;
  File const fOutput;
  je2be::toje::Options fOptions;
  std::weak_ptr<ConvertProgress> fUpdater;
  int const fStepOffset;
};

B2JConvertProgress::B2JConvertProgress(B2JConfigState const &configState) : fConfigState(configState) {
  auto width = kWindowWidth;
  auto height = kWindowHeight;
  setSize(width, height);

  fTempRoot = TemporaryDirectory::EnsureExisting();
  juce::Uuid u;
  File outputDir = fTempRoot.getChildFile(u.toDashedString());
  outputDir.createDirectory();
  fOutputDirectory = outputDir;
}

void B2JConvertProgress::startThread() {
  je2be::toje::Options opt;
  opt.fTempDirectory = PathFromFile(fTempRoot);
  if (fConfigState.fLocalPlayer) {
    juce::Uuid juceUuid = *fConfigState.fLocalPlayer;
    uint8_t data[16];
    std::copy_n(juceUuid.getRawData(), 16, data);
    auto uuid = je2be::Uuid::FromData(data);
    opt.fLocalPlayer = std::make_shared<Uuid>(uuid);
  }
  fThread.reset(new B2JWorkerThread(fConfigState.fInputState.fInput, fOutputDirectory, opt, isUnzipNeeded() ? 1 : 0, weak_from_this()));
  fThread->startThread();
}

void B2JConvertProgress::onCancelButtonClicked() {
  if (fFailure) {
    JUCEApplication::getInstance()->invoke(commands::toChooseBedrockInput, true);
  } else {
    fCommandWhenFinished = commands::toB2JConfig;
  }
}

void B2JConvertProgress ::onFinish() {
  if (fCommandWhenFinished != commands::toChooseJavaOutput && fOutputDirectory.exists()) {
    TemporaryDirectory::QueueDeletingDirectory(fOutputDirectory);
  }
  if (!fFailure) {
    fState = JavaConvertedState(fConfigState.fInputState.fWorldName, fOutputDirectory);
    JUCEApplication::getInstance()->invoke(fCommandWhenFinished, true);
  }
}

} // namespace je2be::desktop::component::b2j
