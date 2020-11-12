#pragma once

#include "ComponentState.h"
#include <JuceHeader.h>

class ChooseOutputComponent : public juce::Component,
                              public FileBrowserListener,
                              public ChooseOutputStateProvider,
                              public ChooseInputStateProvider {
public:
  explicit ChooseOutputComponent(ConvertState const &convertState);
  ~ChooseOutputComponent() override;

  void paint(juce::Graphics &) override;

  void onSaveButtonClicked();
  void onCancelButtonClicked();

  ChooseOutputState getChooseOutputState() const override { return fState; }
  ChooseInputState getChooseInputState() const override {
    return fState.fConvertState.fConfigState.fInputState;
  }

  void selectionChanged() override;
  void fileClicked(const File &file, const MouseEvent &e) override;
  void fileDoubleClicked(const File &file) override;
  void browserRootChanged(const File &newRoot) override;

private:
  std::unique_ptr<TextButton> fSaveButton;
  std::unique_ptr<TextButton> fCancelButton;
  std::unique_ptr<FileListComponent> fListComponent;
  std::unique_ptr<DirectoryContentsList> fList;
  TimeSliceThread fListThread;
  ChooseOutputState fState;
  std::unique_ptr<Label> fMessage;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChooseOutputComponent)
};
