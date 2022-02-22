#pragma once

#include "ComponentState.h"
#include "TextButtonComponent.h"
#include <optional>

namespace je2be::gui::j2b {

class J2BChooseInputComponent : public juce::Component,
                                public J2BChooseInputStateProvider,
                                public juce::FileBrowserListener,
                                public juce::ChangeListener {
public:
  explicit J2BChooseInputComponent(std::optional<J2BChooseInputState> state);
  ~J2BChooseInputComponent() override;

  void paint(juce::Graphics &) override;

  J2BChooseInputState getChooseInputState() const override {
    return fState;
  }

  void selectionChanged() override;
  void fileClicked(const juce::File &file, const juce::MouseEvent &e) override;
  void fileDoubleClicked(const juce::File &file) override;
  void browserRootChanged(const juce::File &newRoot) override;

  void changeListenerCallback(juce::ChangeBroadcaster *source) override;

private:
  void onNextButtonClicked();
  void onChooseCustomButtonClicked();
  void onBackButtonClicked();

  void onCustomDirectorySelected(juce::FileChooser const &chooser);

private:
  static juce::File sLastDirectory;

  std::unique_ptr<TextButtonComponent> fNextButton;
  std::unique_ptr<TextButtonComponent> fChooseCustomButton;
  std::unique_ptr<juce::FileListComponent> fListComponent;
  std::unique_ptr<juce::DirectoryContentsList> fList;
  juce::TimeSliceThread fListThread;
  J2BChooseInputState fState;
  std::unique_ptr<juce::Label> fMessage;
  std::optional<juce::File> fInitialSelection;
  std::unique_ptr<TextButtonComponent> fBackButton;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(J2BChooseInputComponent)
};

} // namespace je2be::gui::j2b
