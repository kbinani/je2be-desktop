#pragma once

#include "ComponentState.h"
#include "GameDirectory.h"

namespace je2be::gui {
class GameDirectoryScanThreadJava;
}

namespace je2be::gui::component {
class TextButton;
}

namespace je2be::gui::component::j2b {

class J2BChooseInput : public juce::Component,
                       public J2BChooseInputStateProvider,
                       public juce::AsyncUpdater,
                       public juce::ListBoxModel {
public:
  explicit J2BChooseInput(std::optional<J2BChooseInputState> state);
  ~J2BChooseInput() override;

  void paint(juce::Graphics &) override;

  J2BChooseInputState getChooseInputState() const override {
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
  std::unique_ptr<GameDirectoryScanThreadJava> fThread;
  J2BChooseInputState fState;
  std::unique_ptr<juce::Label> fMessage;
  std::unique_ptr<TextButton> fBackButton;
  std::vector<GameDirectory> fGameDirectories;
  std::unique_ptr<juce::Label> fPlaceholder;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(J2BChooseInput)
};

} // namespace je2be::gui::component::j2b
