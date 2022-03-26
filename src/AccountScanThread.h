#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace je2be::gui {

struct Account {
  juce::String fName;
  juce::Uuid fUuid;
  juce::String fType;     // Mojang / Xbox
  juce::String fUsername; // Mojang: mail address / Xbox: GamerTag

  juce::String toString() const {
    return fName + " (" + fType + ", " + fUsername + ")";
  }
};

class AccountScanThread : public juce::Thread {
public:
  explicit AccountScanThread(juce::AsyncUpdater *parent);
  ~AccountScanThread();

  void run() override;
  void copyAccounts(std::vector<Account> &buffer);

private:
  class Impl;
  std::unique_ptr<Impl> fImpl;
};

} // namespace je2be::gui
