#pragma once

#include "CommandID.h"
#include "ComponentState.h"
#include "Status.hpp"
#include "component/ConvertProgress.h"

namespace je2be::desktop::component::j2b {

class J2BConvertProgress : public ConvertProgress,
                           public BedrockConvertedStateProvider,
                           public J2BConfigStateProvider,
                           public ChooseInputStateProvider,
                           public std::enable_shared_from_this<J2BConvertProgress> {
public:
  explicit J2BConvertProgress(J2BConfigState const &configState);
  ~J2BConvertProgress() override;

  void paint(juce::Graphics &) override {}

  J2BConfigState getConfigState() const override {
    return fConfigState;
  }

  std::optional<BedrockConvertedState> getConvertedState() const override {
    return fState;
  }

  std::optional<ChooseInputState> getChooseInputState() const override {
    return fConfigState.fInputState;
  }

  void onCancelButtonClicked() override;
  void startThread() override;
  void onFinish() override;

  int getProgressSteps() const override {
    return 3;
  }

  Characteristics getProgressCharacteristics(int step) const override {
    switch (step) {
    case 1:
      return Characteristics(Characteristics::Unit::Percent, 0.4, TRANS("Post processing..."), "PostProcess");
    case 2:
      return Characteristics(Characteristics::Unit::Percent, 0.2, TRANS("LevelDB compaction"), "LevelDB Compaction");
    case 0:
    default:
      return Characteristics(Characteristics::Unit::Chunk, 0.4, TRANS("Converting..."), "Conversion", "Converted %d Chunks: ");
    }
  }

private:
  J2BConfigState fConfigState;
  std::optional<BedrockConvertedState> fState;
  juce::File fTempRoot;
  juce::File fOutputDirectory;
  juce::CommandID fCommandWhenFinished = commands::toChooseBedrockOutput;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(J2BConvertProgress)
};

} // namespace je2be::desktop::component::j2b
