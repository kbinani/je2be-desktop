#pragma once

#include "ComponentState.h"

namespace je2be::gui::component {
class TextButton;
}

namespace je2be::gui::component::b2j {

class B2JChooseOutput : public juce::Component,
                        public B2JChooseOutputStateProvider,
                        public B2JChooseInputStateProvider {
public:
  explicit B2JChooseOutput(B2JConvertState const &convertState);
  ~B2JChooseOutput() override;

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
  std::unique_ptr<TextButton> fBackButton;
  std::unique_ptr<TextButton> fSaveToDefaultDirectory;
  std::unique_ptr<TextButton> fSaveToCustomDirectory;
  juce::File fDefaultSaveDirectory;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(B2JChooseOutput)
};

} // namespace je2be::gui::component::b2j
