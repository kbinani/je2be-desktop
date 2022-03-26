#pragma once

#include "ComponentState.h"
#include "GameDirectory.h"

namespace je2be::desktop {

class GameDirectoryScanThreadXbox360;

}

namespace je2be::desktop::component {

class TextButton;

class ChooseXbox360Input : public juce::Component,
                           public ChooseInputStateProvider,
                           public juce::ListBoxModel,
                           public juce::AsyncUpdater {
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
  std::optional<ChooseInputState> fState;
  std::unique_ptr<juce::Label> fMessage;
  std::unique_ptr<TextButton> fBackButton;
  juce::File fBedrockGameDirectory;
  std::unique_ptr<GameDirectoryScanThreadXbox360> fThread;
  std::unique_ptr<juce::Label> fPlaceholder;
  std::vector<GameDirectory> fGameDirectories;
  juce::CommandID const fDestinationAfterChoose;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChooseXbox360Input)
};

} // namespace je2be::desktop::component
