#pragma once

#include "ComponentState.h"

namespace je2be::gui::component {

class TextButton;

class ChooseBedrockOutput : public juce::Component,
                            public J2BChooseOutputStateProvider,
                            public ChooseInputStateProvider {
public:
  explicit ChooseBedrockOutput(J2BConvertState const &convertState);
  ~ChooseBedrockOutput() override;

  void paint(juce::Graphics &) override;

  J2BChooseOutputState getChooseOutputState() const override {
    return fState;
  }

  std::optional<ChooseInputState> getChooseInputState() const override {
    return fState.fConvertState.fConfigState.fInputState;
  }

  void onBackButtonClicked();

  void onSaveToDefaultButtonClicked();
  void onSaveToCustomButtonClicked();
  void onSaveAsZipButtonClicked();

  void onCustomDestinationDirectorySelected(juce::FileChooser const &chooser);
  void onZipDestinationFileSelected(juce::FileChooser const &chooser);

private:
  static juce::File sLastCustomDirectory;
  static juce::File sLastZipFile;

  J2BChooseOutputState fState;
  std::unique_ptr<juce::Label> fMessage;
  std::unique_ptr<TextButton> fBackButton;
  std::unique_ptr<TextButton> fSaveToDefaultDirectory;
  std::unique_ptr<TextButton> fSaveToCustomDirectory;
  std::unique_ptr<TextButton> fSaveAsZipFile;
  juce::File fDefaultSaveDirectory;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChooseBedrockOutput)
};

} // namespace je2be::gui::component