#pragma once

#include "ComponentState.h"
#include "GameDirectoryScanThreadJava.h"
#include "component/TextButton.h"
#include <optional>

namespace je2be::gui::component::j2b {

class J2BChooseInputComponent : public juce::Component,
                                public J2BChooseInputStateProvider,
                                public juce::AsyncUpdater,
                                public juce::ListBoxModel {
public:
  explicit J2BChooseInputComponent(std::optional<J2BChooseInputState> state);
  ~J2BChooseInputComponent() override;

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

  std::unique_ptr<TextButtonComponent> fNextButton;
  std::unique_ptr<TextButtonComponent> fChooseCustomButton;
  std::unique_ptr<juce::ListBox> fListComponent;
  std::unique_ptr<GameDirectoryScanThreadJava> fThread;
  J2BChooseInputState fState;
  std::unique_ptr<juce::Label> fMessage;
  std::unique_ptr<TextButtonComponent> fBackButton;
  std::vector<GameDirectory> fGameDirectories;
  std::unique_ptr<juce::Label> fPlaceholder;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(J2BChooseInputComponent)
};

} // namespace je2be::gui::j2b
