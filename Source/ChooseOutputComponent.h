#pragma once

#include "ComponentState.h"

class ChooseOutputComponent : public juce::Component,
                              public ChooseOutputStateProvider,
                              public ChooseInputStateProvider {
public:
  explicit ChooseOutputComponent(ConvertState const &convertState);
  ~ChooseOutputComponent() override;

  void paint(juce::Graphics &) override;

  ChooseOutputState getChooseOutputState() const override {
    return fState;
  }

  ChooseInputState getChooseInputState() const override {
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

  ChooseOutputState fState;
  std::unique_ptr<juce::Label> fMessage;
  std::unique_ptr<juce::TextButton> fBackButton;
  std::unique_ptr<juce::TextButton> fSaveToDefaultDirectory;
  std::unique_ptr<juce::TextButton> fSaveToCustomDirectory;
  std::unique_ptr<juce::TextButton> fSaveAsZipFile;
  juce::File fDefaultSaveDirectory;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChooseOutputComponent)
};
