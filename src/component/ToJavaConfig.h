#pragma once

#include "AccountScanThread.h"
#include "CommandID.h"
#include "ComponentState.h"

namespace je2be::desktop::component {

class TextButton;

class ToJavaConfig : public juce::Component,
                     public ChooseInputStateProvider,
                     public ToJavaConfigStateProvider,
                     public juce::Timer,
                     public juce::AsyncUpdater {
public:
  ToJavaConfig(ChooseInputState const &inputState, int forwardCommand, int backwardCommand);
  ~ToJavaConfig() override;

  void paint(juce::Graphics &) override;

  std::optional<ChooseInputState> getChooseInputState() const override {
    return fState.fInputState;
  }

  ToJavaConfigState getConfigState() const override {
    return fState;
  }

  void timerCallback() override;
  void handleAsyncUpdate() override;

private:
  void updateStartButton();

  void onBackButtonClicked();
  void onStartButtonClicked();
  void onClickImportAccountFromLauncherButton();

private:
  std::unique_ptr<TextButton> fBackButton;
  std::unique_ptr<TextButton> fStartButton;
  ToJavaConfigState fState;
  std::unique_ptr<juce::Label> fFileOrDirectory;
  bool fOk = false;
  std::unique_ptr<juce::Label> fMessage;
  std::unique_ptr<juce::ToggleButton> fImportAccountFromLauncher;
  std::unique_ptr<juce::ComboBox> fAccountList;
  std::unique_ptr<juce::Label> fAccountListLabel;
  std::vector<Account> fAccounts;
  std::unique_ptr<AccountScanThread> fAccountScanThread;
  int const fForwardCommand;
  int const fBackwardCommand;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ToJavaConfig)
};

} // namespace je2be::desktop::component
