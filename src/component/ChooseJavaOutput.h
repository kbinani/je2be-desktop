#pragma once

#include "ComponentState.h"

namespace je2be::gui::component {

class TextButton;

class ChooseJavaOutput : public juce::Component,
                         public B2JChooseOutputStateProvider,
                         public ChooseInputStateProvider {
public:
  explicit ChooseJavaOutput(B2JConvertState const &convertState);
  ~ChooseJavaOutput() override;

  void paint(juce::Graphics &) override;

  B2JChooseOutputState getChooseOutputState() const override {
    return fState;
  }

  std::optional<ChooseInputState> getChooseInputState() const override {
    return fState.fConvertState.fConfigState.fInputState;
  }

  void onBackButtonClicked();

  void onSaveToDefaultButtonClicked();
  void onSaveToCustomButtonClicked();

  void onCustomDestinationDirectorySelected(juce::FileChooser const &chooser);

private:
  static juce::File sLastCustomDirectory;
  static juce::File sLastZipFile;

  B2JChooseOutputState fState;
  std::unique_ptr<juce::Label> fMessage;
  std::unique_ptr<TextButton> fBackButton;
  std::unique_ptr<TextButton> fSaveToDefaultDirectory;
  std::unique_ptr<TextButton> fSaveToCustomDirectory;
  juce::File fDefaultSaveDirectory;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChooseJavaOutput)
};

} // namespace je2be::gui::component
