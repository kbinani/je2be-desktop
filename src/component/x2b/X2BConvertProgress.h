#pragma once

#include "CommandID.h"
#include "ComponentState.h"
#include "Status.hpp"
#include "component/ConvertProgress.h"

namespace je2be::desktop::component::x2b {

class X2BConvertProgress : public ConvertProgress,
                           public BedrockConvertedStateProvider,
                           public X2BConfigStateProvider,
                           public ChooseInputStateProvider,
                           public std::enable_shared_from_this<X2BConvertProgress> {
public:
  explicit X2BConvertProgress(X2BConfigState const &configState);
  ~X2BConvertProgress() override {}

  void paint(juce::Graphics &) override {}

  X2BConfigState getConfigState() const override {
    return fConfigState;
  }

  std::optional<BedrockConvertedState> getConvertedState() const override {
    return fState;
  }

  std::optional<ChooseInputState> getChooseInputState() const override {
    return fConfigState.fInputState;
  }

  int getProgressSteps() const override {
    return 4;
  }

  Characteristics getProgressCharacteristics(int step) const override {
    switch (step) {
    case 1:
      return Characteristics(Characteristics::Unit::Chunk, 0.3, TRANS("Converting..."), "Conversion", "Converted %d Chunks: ");
    case 2:
      return Characteristics(Characteristics::Unit::Percent, 0.2, TRANS("PostProcess"), "PostProcess");
    case 3:
      return Characteristics(Characteristics::Unit::Percent, 0.167, TRANS("LevelDB compaction"), "LevelDB Compaction");
    case 0:
    default:
      return Characteristics(Characteristics::Unit::Percent, 0.333, TRANS("Extracting..."), "Extraction");
    }
  }

  void onCancelButtonClicked() override;
  void startThread() override;
  void onFinish() override;

private:
  X2BConfigState fConfigState;
  std::optional<BedrockConvertedState> fState;
  juce::File fOutputDirectory;
  juce::CommandID fCommandWhenFinished = commands::toChooseBedrockOutput;
  juce::File fTempRoot;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(X2BConvertProgress)
};

} // namespace je2be::desktop::component::x2b
