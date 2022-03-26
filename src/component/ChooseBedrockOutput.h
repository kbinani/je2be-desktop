#pragma once

#include "ComponentState.h"

namespace je2be::gui::component {

class TextButton;

class ChooseBedrockOutput : public juce::Component,
                            public BedrockOutputChoosenStateProvider {
public:
  explicit ChooseBedrockOutput(BedrockConvertedState const &convertState);
  ~ChooseBedrockOutput() override;

  void paint(juce::Graphics &) override;

  BedrockOutputChoosenState getBedrockOutputChoosenState() const override {
    return fState;
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

  BedrockOutputChoosenState fState;
  std::unique_ptr<juce::Label> fMessage;
  std::unique_ptr<TextButton> fBackButton;
  std::unique_ptr<TextButton> fSaveToDefaultDirectory;
  std::unique_ptr<TextButton> fSaveToCustomDirectory;
  std::unique_ptr<TextButton> fSaveAsZipFile;
  juce::File fDefaultSaveDirectory;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChooseBedrockOutput)
};

} // namespace je2be::gui::component
