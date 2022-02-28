#pragma once

#include "ComponentState.h"
#include "component/TextButton.h"

namespace je2be::gui::component::j2b {

class J2BChooseOutputComponent : public juce::Component,
                                 public J2BChooseOutputStateProvider,
                                 public J2BChooseInputStateProvider {
public:
  explicit J2BChooseOutputComponent(J2BConvertState const &convertState);
  ~J2BChooseOutputComponent() override;

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
  std::unique_ptr<TextButtonComponent> fBackButton;
  std::unique_ptr<TextButtonComponent> fSaveToDefaultDirectory;
  std::unique_ptr<TextButtonComponent> fSaveToCustomDirectory;
  std::unique_ptr<TextButtonComponent> fSaveAsZipFile;
  juce::File fDefaultSaveDirectory;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(J2BChooseOutputComponent)
};

} // namespace je2be::gui::j2b
