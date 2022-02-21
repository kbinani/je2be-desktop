#pragma once

#include "ComponentState.h"

namespace je2be::gui::b2j {

class B2JConfigComponent : public juce::Component,
                           public B2JChooseInputStateProvider,
                           public B2JConfigStateProvider,
                           public juce::Timer,
                           public juce::AsyncUpdater {
public:
  explicit B2JConfigComponent(B2JChooseInputState const &inputState);
  ~B2JConfigComponent() override;

  void paint(juce::Graphics &) override;

  B2JChooseInputState getChooseInputState() const override {
    return fState.fInputState;
  }

  B2JConfigState getConfigState() const override {
    return fState;
  }

  void timerCallback() override;
  void handleAsyncUpdate() override;

  struct Account {
    juce::String fName;
    juce::Uuid fUuid;
  };

  class ImportAccountWorker : public juce::Thread {
  public:
    explicit ImportAccountWorker(B2JConfigComponent *parent);
    ~ImportAccountWorker();

    void run() override;
    void copyAccounts(std::vector<Account> &buffer);

  private:
    class Impl;
    std::unique_ptr<Impl> fImpl;
  };

private:
  void onBackButtonClicked();
  void onStartButtonClicked();
  void onClickImportAccountFromLauncherButton();

private:
  std::unique_ptr<juce::TextButton> fBackButton;
  std::unique_ptr<juce::TextButton> fStartButton;
  B2JConfigState fState;
  std::unique_ptr<juce::Label> fFileOrDirectory;
  bool fOk = false;
  std::unique_ptr<juce::Label> fMessage;
  std::unique_ptr<juce::ToggleButton> fImportAccountFromLauncher;
  std::unique_ptr<juce::ComboBox> fAccountList;
  std::vector<Account> fAccounts;
  std::unique_ptr<ImportAccountWorker> fImportAccountWorker;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(B2JConfigComponent)
};

} // namespace je2be::gui::b2j
