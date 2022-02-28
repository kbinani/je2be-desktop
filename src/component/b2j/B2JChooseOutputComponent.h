#pragma once

#include "ComponentState.h"
#include "TextButtonComponent.h"

namespace je2be::gui::component::b2j {

class B2JChooseOutputComponent : public juce::Component,
                                 public B2JChooseOutputStateProvider,
                                 public B2JChooseInputStateProvider {
public:
  explicit B2JChooseOutputComponent(B2JConvertState const &convertState);
  ~B2JChooseOutputComponent() override;

  void paint(juce::Graphics &) override;

  B2JChooseOutputState getChooseOutputState() const override {
    return fState;
  }

  B2JChooseInputState getChooseInputState() const override {
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
  std::unique_ptr<TextButtonComponent> fBackButton;
  std::unique_ptr<TextButtonComponent> fSaveToDefaultDirectory;
  std::unique_ptr<TextButtonComponent> fSaveToCustomDirectory;
  juce::File fDefaultSaveDirectory;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(B2JChooseOutputComponent)
};

} // namespace je2be::gui::b2j
