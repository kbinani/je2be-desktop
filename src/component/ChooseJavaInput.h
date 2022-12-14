#pragma once

#include "ComponentState.h"
#include "GameDirectory.h"

namespace je2be::desktop {
class GameDirectoryScanThreadJava;
}

namespace je2be::desktop::component {

class TextButton;
class SearchLabel;

class ChooseJavaInput : public juce::Component,
                        public ChooseInputStateProvider,
                        public juce::AsyncUpdater,
                        public juce::ListBoxModel {
public:
  explicit ChooseJavaInput(std::optional<ChooseInputState> state);
  ~ChooseJavaInput() override;

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
  void onChooseCustomButtonClicked();
  void onBackButtonClicked();

  void onCustomDirectorySelected(juce::FileChooser const &chooser);

  void onSearchTextChanged();
  void updateGameDirectoriesVisible();

private:
  static juce::File sLastDirectory;

  std::unique_ptr<TextButton> fNextButton;
  std::unique_ptr<TextButton> fChooseCustomButton;
  std::unique_ptr<juce::ListBox> fListComponent;
  std::unique_ptr<GameDirectoryScanThreadJava> fThread;
  std::optional<ChooseInputState> fState;
  std::unique_ptr<juce::Label> fMessage;
  std::unique_ptr<juce::Label> fOrMessage;
  std::unique_ptr<TextButton> fBackButton;
  std::vector<GameDirectory> fGameDirectoriesVisible;
  std::vector<GameDirectory> fGameDirectoriesAll;
  std::unique_ptr<juce::Label> fPlaceholder;
  std::unique_ptr<SearchLabel> fSearch;
  std::unique_ptr<juce::Label> fSearchPlaceholder;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChooseJavaInput)
};

} // namespace je2be::desktop::component
