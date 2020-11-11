#pragma once

#include "ComponentState.h"
#include <JuceHeader.h>

class ChooseInputComponent : public juce::Component,
                             public ChooseInputStateProvider,
                             public FileBrowserListener {
public:
  ChooseInputComponent();
  ~ChooseInputComponent() override;

  void paint(juce::Graphics &) override;
  void resized() override;

  ChooseInputState getChooseInputState() const override { return fState; }

  void selectionChanged() override;
  void fileClicked(const File &file, const MouseEvent &e) override;
  void fileDoubleClicked(const File &file) override;
  void browserRootChanged(const File &newRoot) override;

private:
  void onNextButtonClicked();
  void onChooseCustomButtonClicked();

private:
  std::unique_ptr<TextButton> fNextButton;
  std::unique_ptr<TextButton> fChooseCustomButton;
  std::unique_ptr<FileListComponent> fListComponent;
  std::unique_ptr<DirectoryContentsList> fList;
  TimeSliceThread fListThread;
  ChooseInputState fState;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChooseInputComponent)
};
