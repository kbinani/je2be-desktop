#pragma once

#include "AsyncUpdaterWith.h"
#include "ComponentState.h"
#include "GameDirectory.h"

namespace je2be::desktop {

class GameDirectoryScanWorkerXbox360;

}

namespace je2be::desktop::component {

class TextButton;
class SearchLabel;

class ChooseXbox360Input : public juce::Component,
                           public ChooseInputStateProvider,
                           public juce::ListBoxModel,
                           public AsyncUpdaterWith<std::vector<GameDirectory>>,
                           public std::enable_shared_from_this<ChooseXbox360Input> {
public:
  ChooseXbox360Input(juce::CommandID desinationAfterChoose, std::optional<ChooseInputState> state);
  ~ChooseXbox360Input() override;

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
  void parentHierarchyChanged() override;

  void handleAsyncUpdateWith(std::vector<GameDirectory>) override;

private:
  void onNextButtonClicked();
  void onChooseCustomButtonClicked();
  void onBackButtonClicked();

  void onCustomDirectorySelected(juce::FileChooser const &chooser);

  void onSearchTextChanged();

private:
  static juce::File sLastDirectory;

  std::unique_ptr<TextButton> fNextButton;
  std::unique_ptr<TextButton> fChooseCustomButton;
  std::unique_ptr<juce::ListBox> fListComponent;
  std::optional<ChooseInputState> fState;
  std::unique_ptr<juce::Label> fMessage;
  std::unique_ptr<juce::Label> fOrMessage;
  std::unique_ptr<TextButton> fBackButton;
  juce::File fBedrockGameDirectory;
  std::weak_ptr<GameDirectoryScanWorkerXbox360> fWorker;
  bool fWorkerStarted = false;
  std::unique_ptr<juce::Label> fPlaceholder;
  std::vector<GameDirectory> fGameDirectoriesVisible;
  std::vector<GameDirectory> fGameDirectoriesAll;
  juce::CommandID const fDestinationAfterChoose;
  std::unique_ptr<SearchLabel> fSearch;
  std::unique_ptr<juce::Label> fSearchPlaceholder;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChooseXbox360Input)
};

} // namespace je2be::desktop::component
