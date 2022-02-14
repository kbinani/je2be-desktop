#pragma once

#include "ComponentState.h"

namespace je2be::gui {

class ChooseOutputComponent : public juce::Component,
                              public J2BChooseOutputStateProvider,
                              public J2BChooseInputStateProvider {
public:
  explicit ChooseOutputComponent(J2BConvertState const &convertState);
  ~ChooseOutputComponent() override;

  void paint(juce::Graphics &) override;

  J2BChooseOutputState getChooseOutputState() const override {
    return fState;
  }

  J2BChooseInputState getChooseInputState() const override {
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
  std::unique_ptr<juce::TextButton> fBackButton;
  std::unique_ptr<juce::TextButton> fSaveToDefaultDirectory;
  std::unique_ptr<juce::TextButton> fSaveToCustomDirectory;
  std::unique_ptr<juce::TextButton> fSaveAsZipFile;
  juce::File fDefaultSaveDirectory;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChooseOutputComponent)
};

} // namespace je2be::gui
