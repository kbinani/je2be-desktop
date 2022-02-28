#pragma once

#include "ComponentState.h"
#include "GameDirectory.h"
#include "GameDirectoryScanThreadBedrock.h"
#include "component/TextButton.h"
#include <optional>

namespace je2be::gui::component::b2j {

class B2JChooseInput : public juce::Component,
                       public B2JChooseInputStateProvider,
                       public juce::ListBoxModel,
                       public juce::AsyncUpdater {
public:
  explicit B2JChooseInput(std::optional<B2JChooseInputState> state);
  ~B2JChooseInput() override;

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

  void handleAsyncUpdate() override;

private:
  void onNextButtonClicked();
  void onChooseCustomButtonClicked();
  void onBackButtonClicked();

  void onCustomDirectorySelected(juce::FileChooser const &chooser);

private:
  static juce::File sLastDirectory;

  std::unique_ptr<TextButton> fNextButton;
  std::unique_ptr<TextButton> fChooseCustomButton;
  std::unique_ptr<juce::ListBox> fListComponent;
  B2JChooseInputState fState;
  std::unique_ptr<juce::Label> fMessage;
  std::unique_ptr<TextButton> fBackButton;
  juce::File fBedrockGameDirectory;
  std::unique_ptr<GameDirectoryScanThreadBedrock> fThread;
  std::unique_ptr<juce::Label> fPlaceholder;

  std::vector<GameDirectory> fGameDirectories;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(B2JChooseInput)
};

} // namespace je2be::gui::component::b2j
