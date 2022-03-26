#pragma once

#include "AccountScanThread.h"
#include "ComponentState.h"

namespace je2be::desktop::component {
class TextButton;
}

namespace je2be::desktop::component::b2j {

class B2JConfig : public juce::Component,
                  public ChooseInputStateProvider,
                  public B2JConfigStateProvider,
                  public juce::Timer,
                  public juce::AsyncUpdater {
public:
  explicit B2JConfig(ChooseInputState const &inputState);
  ~B2JConfig() override;

  void paint(juce::Graphics &) override;

  std::optional<ChooseInputState> getChooseInputState() const override {
    return fState.fInputState;
  }

  B2JConfigState getConfigState() const override {
    return fState;
  }

  void timerCallback() override;
  void handleAsyncUpdate() override;

private:
  void onBackButtonClicked();
  void onStartButtonClicked();
  void onClickImportAccountFromLauncherButton();

private:
  std::unique_ptr<TextButton> fBackButton;
  std::unique_ptr<TextButton> fStartButton;
  B2JConfigState fState;
  std::unique_ptr<juce::Label> fFileOrDirectory;
  bool fOk = false;
  std::unique_ptr<juce::Label> fMessage;
  std::unique_ptr<juce::ToggleButton> fImportAccountFromLauncher;
  std::unique_ptr<juce::ComboBox> fAccountList;
  std::unique_ptr<juce::Label> fAccountListLabel;
  std::vector<Account> fAccounts;
  std::unique_ptr<AccountScanThread> fAccountScanThread;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(B2JConfig)
};

} // namespace je2be::desktop::component::b2j
