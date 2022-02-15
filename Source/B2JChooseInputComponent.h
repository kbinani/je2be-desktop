#pragma once

#include "ComponentState.h"
#include <optional>

namespace je2be::gui::b2j {

class B2JChooseInputComponent : public juce::Component,
                                public B2JChooseInputStateProvider,
                                public juce::ListBoxModel {
public:
  explicit B2JChooseInputComponent(std::optional<B2JChooseInputState> state);
  ~B2JChooseInputComponent() override;

  void paint(juce::Graphics &) override;

  B2JChooseInputState getChooseInputState() const override {
    return fState;
  }

  int getNumRows() override;
  void paintListBoxItem(int rowNumber,
                        juce::Graphics &g,
                        int width, int height,
                        bool rowIsSelected) override;
  void selectedRowsChanged(int lastRowSelected);
  void listBoxItemDoubleClicked(int row, const juce::MouseEvent &);

  struct GameDirectory {
    juce::File fDirectory;
    juce::String fLevelName;
  };

private:
  void onNextButtonClicked();
  void onChooseCustomButtonClicked();
  void onBackButtonClicked();

  void onCustomDirectorySelected(juce::FileChooser const &chooser);

private:
  static juce::File sLastDirectory;

  std::unique_ptr<juce::TextButton> fNextButton;
  std::unique_ptr<juce::TextButton> fChooseCustomButton;
  std::unique_ptr<juce::ListBox> fListComponent;
  B2JChooseInputState fState;
  std::unique_ptr<juce::Label> fMessage;
  std::unique_ptr<juce::TextButton> fBackButton;
  juce::File fBedrockGameDirectory;

  std::vector<GameDirectory> fGameDirectories;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(B2JChooseInputComponent)
};

} // namespace je2be::gui::b2j
