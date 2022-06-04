#pragma once

#include "AccountScanThread.h"
#include "ComponentState.h"

namespace je2be::desktop::component {
class TextButton;
}

namespace je2be::desktop::component::x2j {

class X2JConfig : public juce::Component,
                  public ChooseInputStateProvider,
                  public X2JConfigStateProvider,
                  public juce::Timer,
                  public juce::AsyncUpdater {
public:
  explicit X2JConfig(ChooseInputState const &inputState);
  ~X2JConfig() override;

  void paint(juce::Graphics &) override;

  std::optional<ChooseInputState> getChooseInputState() const override {
    return fState.fInputState;
  }

  X2JConfigState getConfigState() const override {
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
  X2JConfigState fState;
  std::unique_ptr<juce::Label> fFileOrDirectory;
  bool fOk = false;
  std::unique_ptr<juce::Label> fMessage;
  std::unique_ptr<juce::ToggleButton> fImportAccountFromLauncher;
  std::unique_ptr<juce::ComboBox> fAccountList;
  std::unique_ptr<juce::Label> fAccountListLabel;
  std::vector<Account> fAccounts;
  std::unique_ptr<AccountScanThread> fAccountScanThread;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(X2JConfig)
};

} // namespace je2be::desktop::component::x2j
