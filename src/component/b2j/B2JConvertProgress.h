#pragma once

#include "CommandID.h"
#include "ComponentState.h"
#include "Status.hpp"
#include "component/ConvertProgress.h"

namespace je2be::desktop::component::b2j {

class B2JConvertProgress : public ConvertProgress,
                           public JavaConvertedStateProvider,
                           public ToJavaConfigStateProvider,
                           public ChooseInputStateProvider,
                           public std::enable_shared_from_this<B2JConvertProgress> {
public:
  explicit B2JConvertProgress(ToJavaConfigState const &configState);
  ~B2JConvertProgress() override {}

  void paint(juce::Graphics &) override {}

  ToJavaConfigState getConfigState() const override {
    return fConfigState;
  }

  std::optional<JavaConvertedState> getConvertedState() const override {
    return fState;
  }

  std::optional<ChooseInputState> getChooseInputState() const override {
    return fConfigState.fInputState;
  }

  int getProgressSteps() const override {
    if (isUnzipNeeded()) {
      return 3;
    } else {
      return 2;
    }
  }

  Characteristics getProgressCharacteristics(int step) const override {
    // unzip:        0:02.46 =   2.46
    // conversion:   2:13.28 = 133.28
    // post-process: 4:56.09 = 296.09
    if (isUnzipNeeded()) {
      // 431.83
      switch (step) {
      case 1:
        return Characteristics(Characteristics::Unit::Chunk, 0.309, TRANS("Converting..."), "Conversion", "Converted %d Chunks: ");
      case 2:
        return Characteristics(Characteristics::Unit::Chunk, 0.686, TRANS("Post processing..."), "Post process", "Post processed %d Chunks: ");
      case 0:
      default:
        return Characteristics(Characteristics::Unit::Percent, 0.005, TRANS("Unzipping..."), "Unzip");
      }
    } else {
      // 429.37
      switch (step) {
      case 1:
        return Characteristics(Characteristics::Unit::Chunk, 0.69, TRANS("Post processing..."), "Post process", "Post processed %d Chunks: ");
      case 0:
      default: {
        return Characteristics(Characteristics::Unit::Chunk, 0.31, TRANS("Converting..."), "Conversion", "Converted %d Chunks: ");
      }
      }
    }
  }

  void onCancelButtonClicked() override;
  void startThread() override;
  void onFinish() override;

private:
  ToJavaConfigState const fConfigState;
  juce::File fOutputDirectory;
  std::optional<JavaConvertedState> fState;
  juce::CommandID fCommandWhenFinished = commands::toChooseJavaOutput;
  juce::File fTempRoot;

  bool isUnzipNeeded() const {
    return !fConfigState.fInputState.fInput.isDirectory();
  }

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(B2JConvertProgress)
};

} // namespace je2be::desktop::component::b2j
