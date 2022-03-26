#pragma once

#include "ComponentState.h"

namespace je2be::gui::component {

class TextButton;

class ChooseJavaOutput : public juce::Component,
                         public JavaOutputChoosenStateProvider {
public:
  explicit ChooseJavaOutput(JavaConvertedState const &convertedState);
  ~ChooseJavaOutput() override;

  void paint(juce::Graphics &) override;

  JavaOutputChoosenState getJavaOutputChoosenState() const override {
    return fState;
  }

  void onBackButtonClicked();

  void onSaveToDefaultButtonClicked();
  void onSaveToCustomButtonClicked();

  void onCustomDestinationDirectorySelected(juce::FileChooser const &chooser);

private:
  static juce::File sLastCustomDirectory;
  static juce::File sLastZipFile;

  JavaOutputChoosenState fState;
  std::unique_ptr<juce::Label> fMessage;
  std::unique_ptr<TextButton> fBackButton;
  std::unique_ptr<TextButton> fSaveToDefaultDirectory;
  std::unique_ptr<TextButton> fSaveToCustomDirectory;
  juce::File fDefaultSaveDirectory;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChooseJavaOutput)
};

} // namespace je2be::gui::component
