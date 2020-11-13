#pragma once

#include "ComponentState.h"
#include <JuceHeader.h>

class ChooseInputComponent : public juce::Component,
                             public ChooseInputStateProvider,
                             public FileBrowserListener,
                             public ChangeListener {
public:
  explicit ChooseInputComponent(std::optional<ChooseInputState> state);
  ~ChooseInputComponent() override;

  void paint(juce::Graphics &) override;

  ChooseInputState getChooseInputState() const override { return fState; }

  void selectionChanged() override;
  void fileClicked(const File &file, const MouseEvent &e) override;
  void fileDoubleClicked(const File &file) override;
  void browserRootChanged(const File &newRoot) override;

  void changeListenerCallback(ChangeBroadcaster *source) override;

private:
  void onNextButtonClicked();
  void onChooseCustomButtonClicked();
  void onAboutButtonClicked();

private:
  std::unique_ptr<TextButton> fNextButton;
  std::unique_ptr<TextButton> fChooseCustomButton;
  std::unique_ptr<FileListComponent> fListComponent;
  std::unique_ptr<DirectoryContentsList> fList;
  TimeSliceThread fListThread;
  ChooseInputState fState;
  std::unique_ptr<Label> fMessage;
  std::optional<File> fInitialSelection;
  std::unique_ptr<TextButton> fAboutButton;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChooseInputComponent)
};
