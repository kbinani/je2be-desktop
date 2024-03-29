#include <je2be.hpp>

#include <defer.hpp>

#include "CommandID.h"
#include "Constants.h"
#include "File.h"
#include "TemporaryDirectory.h"
#include "component/p2j/P2JConvertProgress.h"

using namespace juce;

namespace je2be::desktop::component::p2j {

class P2JWorkerThread : public Thread, public je2be::lce::Progress {
public:
  P2JWorkerThread(File input, File output, je2be::lce::Options opt, std::weak_ptr<ConvertProgress> updater)
      : Thread("je2be::desktop::component::p2j::P2JWorkerThread"), fInput(input), fOutput(output), fOptions(opt), fUpdater(updater) {}

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
    File sessionTempDir = TemporaryDirectory::EnsureExisting();
    juce::Uuid u;
    File temp = sessionTempDir.getChildFile(u.toDashedString());
    if (auto st = temp.createDirectory(); !st.ok()) {
      triggerError(Error(__FILE__, __LINE__, st.getErrorMessage().toStdString()));
      return;
    }
    defer {
      TemporaryDirectory::QueueDeletingDirectory(temp);
    };
    auto status = je2be::ps3::Converter::Run(PathFromFile(fInput), PathFromFile(fOutput), std::thread::hardware_concurrency(), fOptions, this);
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

  bool report(Rational<u64> const &progress) override {
    if (auto updater = fUpdater.lock(); updater) {
      ConvertProgress::Progress p;
      p.fProgress = progress.toD();
      p.fCount = 0;
      updater->notifyProgress(0, p);
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
  je2be::lce::Options fOptions;
  std::weak_ptr<ConvertProgress> fUpdater;
};

P2JConvertProgress::P2JConvertProgress(ToJavaConfigState const &configState) : fConfigState(configState) {
  setSize(kWindowWidth, kWindowHeight);

  fTempRoot = TemporaryDirectory::EnsureExisting();
  juce::Uuid u;
  File outputDir = fTempRoot.getChildFile(u.toDashedString());
  outputDir.createDirectory();
  fOutputDirectory = outputDir;
}

void P2JConvertProgress::startThread() {
  je2be::lce::Options opt;
  opt.fTempDirectory = PathFromFile(fTempRoot);
  if (fConfigState.fLocalPlayer) {
    juce::Uuid juceUuid = *fConfigState.fLocalPlayer;
    uint8_t data[16];
    std::copy_n(juceUuid.getRawData(), 16, data);
    auto uuid = je2be::Uuid::FromData(data);
    opt.fLocalPlayer = uuid;
  }
  fThread.reset(new P2JWorkerThread(fConfigState.fInputState.fInput, fOutputDirectory, opt, weak_from_this()));
  fThread->startThread();
}

void P2JConvertProgress::onCancelButtonClicked() {
  if (fFailure) {
    JUCEApplication::getInstance()->invoke(commands::toChoosePS3InputToJava, true);
  } else {
    fCommandWhenFinished = commands::toPS3ToJavaConfig;
  }
}

void P2JConvertProgress::onFinish() {
  fState = JavaConvertedState(fConfigState.fInputState.fWorldName, fOutputDirectory);
  if (fCommandWhenFinished != commands::toChooseJavaOutput && fOutputDirectory.exists()) {
    TemporaryDirectory::QueueDeletingDirectory(fOutputDirectory);
  }
  if (!fFailure) {
    JUCEApplication::getInstance()->invoke(fCommandWhenFinished, true);
  }
}

} // namespace je2be::desktop::component::p2j
