#pragma once

#include "CommandID.h"
#include "ComponentState.h"
#include "Status.hpp"
#include "component/ConvertProgress.h"

namespace je2be::desktop::component::x2j {

class X2JConvertProgress : public ConvertProgress,
                           public JavaConvertedStateProvider,
                           public ToJavaConfigStateProvider,
                           public ChooseInputStateProvider,
                           public std::enable_shared_from_this<X2JConvertProgress> {
public:
  explicit X2JConvertProgress(ToJavaConfigState const &configState);
  ~X2JConvertProgress() override {}

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
    return 1;
  }

  Characteristics getProgressCharacteristics(int step) const override {
    return Characteristics(Characteristics::Unit::Percent, 1, TRANS("Converting..."), "Conversion");
  }

  void onCancelButtonClicked() override;
  void startThread() override;
  void onFinish() override;

private:
  ToJavaConfigState fConfigState;
  std::optional<JavaConvertedState> fState;
  juce::File fOutputDirectory;
  juce::CommandID fCommandWhenFinished = commands::toChooseJavaOutput;
  juce::File fTempRoot;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(X2JConvertProgress)
};

} // namespace je2be::desktop::component::x2j
