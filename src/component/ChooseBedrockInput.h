#pragma once

#include "ComponentState.h"
#include "GameDirectory.h"

namespace je2be::desktop {
class GameDirectoryScanThreadBedrock;
}

namespace je2be::desktop::component {

class TextButton;
class SearchLabel;

class ChooseBedrockInput : public juce::Component,
                           public ChooseInputStateProvider,
                           public juce::ListBoxModel,
                           public juce::AsyncUpdater {
public:
  explicit ChooseBedrockInput(std::optional<ChooseInputState> state);
  ~ChooseBedrockInput() override;

  void paint(juce::Graphics &) override;

  std::optional<ChooseInputState> getChooseInputState() const override {
    return fState;
  }

  int getNumRows() override;
  void paintListBoxItem(int rowNumber,
                        juce::Graphics &g,
                        int width, int height,
                        bool rowIsSelected) override;
  void selectedRowsChanged(int lastRowSelected) override;
  void listBoxItemDoubleClicked(int row, juce::MouseEvent const &) override;
  void listBoxItemClicked(int row, juce::MouseEvent const &) override;

  void handleAsyncUpdate() override;

private:
  void onNextButtonClicked();
  void onChooseMcworldFileButtonClicked();
  void onChooseDirectoryButtonClicked();
  void onBackButtonClicked();

  void onMcworldFileSelected(juce::FileChooser const &chooser);
  void onDirectorySelected(juce::FileChooser const &chooser);

  void onSearchTextChanged();
  void updateGameDirectoriesVisible();

private:
  static juce::File sLastDirectory;

  std::unique_ptr<TextButton> fNextButton;
  std::unique_ptr<TextButton> fChooseMcworldFileButton;
  std::unique_ptr<TextButton> fChooseDirectoryButton;
  std::unique_ptr<juce::ListBox> fListComponent;
  std::optional<ChooseInputState> fState;
  std::unique_ptr<juce::Label> fMessage;
  std::unique_ptr<juce::Label> fOrMessage1;
  std::unique_ptr<juce::Label> fOrMessage2;
  std::unique_ptr<TextButton> fBackButton;
  juce::File fBedrockGameDirectory;
  std::unique_ptr<GameDirectoryScanThreadBedrock> fThread;
  std::unique_ptr<juce::Label> fPlaceholder;
  std::unique_ptr<SearchLabel> fSearch;
  std::unique_ptr<juce::Label> fSearchPlaceholder;

  std::vector<GameDirectory> fGameDirectoriesVisible;
  std::vector<GameDirectory> fGameDirectoriesAll;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChooseBedrockInput)
};

} // namespace je2be::desktop::component
