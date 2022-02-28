#include "component/b2j/B2JConfig.h"
#include "CommandID.h"
#include "Constants.h"
#include "GameDirectory.h"
#include <je2be.hpp>

using namespace juce;

namespace je2be::gui::component::b2j {

B2JConfig::B2JConfig(B2JChooseInputState const &chooseInputState) : fState(chooseInputState) {
  auto width = kWindowWidth;
  auto height = kWindowHeight;
  setSize(width, height);

  int y = kMargin;
  juce::String label = (*fState.fInputState.fInputFileOrDirectory).getFullPathName();
  fFileOrDirectory.reset(new Label("", TRANS("Selected world:") + " " + label));
  fFileOrDirectory->setBounds(kMargin, kMargin, width - kMargin * 2, kButtonBaseHeight);
  fFileOrDirectory->setJustificationType(Justification::topLeft);
  addAndMakeVisible(*fFileOrDirectory);

  {
    y += 3 * kMargin;
    fImportAccountFromLauncher.reset(new ToggleButton(TRANS("Import player ID from the Minecraft Launcher to embed it into level.dat as a local player ID")));
    fImportAccountFromLauncher->setBounds(kMargin, y, width - kMargin * 2, kButtonBaseHeight);
    fImportAccountFromLauncher->setMouseCursor(MouseCursor::PointingHandCursor);
    fImportAccountFromLauncher->onClick = [this] {
      onClickImportAccountFromLauncherButton();
    };
    addAndMakeVisible(*fImportAccountFromLauncher);
    y += fImportAccountFromLauncher->getHeight();
  }

  {
    int x = 42;

    juce::String title = TRANS("Account:") + " ";
    fAccountListLabel.reset(new Label("", title));
    int labelWidth = getLookAndFeel().getLabelFont(*fAccountListLabel).getStringWidth(title) + 10;
    fAccountListLabel->setBounds(x, y, labelWidth, kButtonBaseHeight);
    fAccountListLabel->setJustificationType(Justification::centredLeft);
    addAndMakeVisible(*fAccountListLabel);

    fAccountList.reset(new ComboBox());
    fAccountList->setBounds(x + labelWidth, y, width - x - labelWidth - kMargin, kButtonBaseHeight);
    fAccountList->setEnabled(false);
    addAndMakeVisible(*fAccountList);
    y += fAccountList->getHeight();
  }

  int messageComponentY = y + kMargin;

  fStartButton.reset(new component::TextButton(TRANS("Start")));
  fStartButton->setBounds(width - kMargin - kButtonMinWidth, height - kMargin - kButtonBaseHeight, kButtonMinWidth, kButtonBaseHeight);
  fStartButton->setEnabled(false);
  fStartButton->onClick = [this]() { onStartButtonClicked(); };
  addAndMakeVisible(*fStartButton);

  fBackButton.reset(new component::TextButton(TRANS("Back")));
  fBackButton->setBounds(kMargin, height - kMargin - kButtonBaseHeight, kButtonMinWidth, kButtonBaseHeight);
  fBackButton->onClick = [this]() { onBackButtonClicked(); };
  addAndMakeVisible(*fBackButton);

  // TODO: check given file or directory
  fOk = true;

  if (fOk) {
    fMessage.reset(new Label("", ""));
  } else {
    fMessage.reset(new Label("", TRANS("There doesn't seem to be any Minecraft save data "
                                       "in the specified directory.")));
    fMessage->setColour(Label::textColourId, kErrorTextColor);
  }
  fMessage->setBounds(kMargin, messageComponentY, width - 2 * kMargin, kButtonBaseHeight);
  addAndMakeVisible(*fMessage);

  startTimer(1000);
}

B2JConfig::~B2JConfig() {}

void B2JConfig::timerCallback() {
  stopTimer();
  fStartButton->setEnabled(fOk);
}

void B2JConfig::paint(juce::Graphics &g) {}

void B2JConfig::onStartButtonClicked() {
  if (fImportAccountFromLauncher->getToggleState()) {
    int selected = fAccountList->getSelectedId();
    int index = selected - 1;
    if (0 <= index && index < fAccounts.size()) {
      Account a = fAccounts[index];
      fState.fLocalPlayer = a.fUuid;
    }
  }
  JUCEApplication::getInstance()->invoke(gui::toB2JConvert, true);
}

void B2JConfig::onBackButtonClicked() {
  JUCEApplication::getInstance()->invoke(gui::toB2JChooseInput, true);
}

class B2JConfig::ImportAccountWorker::Impl {
public:
  explicit Impl(B2JConfig *parent) : fParent(parent) {}

  void run() {
    try {
      unsafeRun();
    } catch (...) {
    }
    fParent->triggerAsyncUpdate();
  }

  void unsafeRun() {
    using namespace std;
    File saves = GameDirectory::JavaSaveDirectory();
    File jsonFile = saves.getParentDirectory().getChildFile("launcher_accounts.json");
    if (!jsonFile.existsAsFile()) {
      return;
    }
    juce::String jsonString = jsonFile.loadFileAsString();
    string jsonStdString(jsonString.toUTF8(), jsonString.getNumBytesAsUTF8());
    auto json = nlohmann::json::parse(jsonStdString);
    auto accounts = json["accounts"];
    unordered_map<string, Account> collected;
    for (auto const &it : accounts) {
      try {
        auto profile = it["minecraftProfile"];
        auto id = profile["id"].get<string>();
        auto name = profile["name"].get<string>();
        auto localId = it["localId"].get<string>();
        auto type = it["type"].get<string>();
        auto username = it["username"].get<string>();
        juce::Uuid uuid(id);
        Account account;
        account.fName = name;
        account.fUuid = uuid;
        account.fType = type;
        account.fUsername = username;
        collected[localId] = account;
      } catch (...) {
      }
    }
    if (collected.empty()) {
      return;
    }
    if (collected.size() == 1) {
      fAccounts.push_back(collected.begin()->second);
      return;
    }
    string activeAccountLocalId;
    try {
      activeAccountLocalId = json["activeAccountLocalId"].get<string>();
    } catch (...) {
    }
    auto first = collected.find(activeAccountLocalId);
    std::optional<Account> preferred;
    if (first != collected.end()) {
      fAccounts.push_back(first->second);
      preferred = first->second;
      collected.erase(first);
    }
    HashMap<juce::Uuid, Account> accountById;
    if (preferred) {
      accountById[preferred->fUuid] = *preferred;
    }
    for (auto const &it : collected) {
      if (accountById.contains(it.second.fUuid)) {
        if (it.second.fType == "Xbox") {
          accountById[it.second.fUuid] = it.second;
        }
      } else {
        accountById[it.second.fUuid] = it.second;
      }
    }
    if (preferred) {
      accountById.remove(preferred->fUuid);
    }
    for (auto const &it : accountById) {
      fAccounts.push_back(it);
    }
  }

  B2JConfig *const fParent;
  std::vector<B2JConfig::Account> fAccounts;
};

B2JConfig::ImportAccountWorker::ImportAccountWorker(B2JConfig *parent) : Thread("je2be::gui::B2JConfigComponent::ImportAccountWorker"), fImpl(new Impl(parent)) {}

B2JConfig::ImportAccountWorker::~ImportAccountWorker() {}

void B2JConfig::ImportAccountWorker::run() {
  fImpl->run();
}

void B2JConfig::ImportAccountWorker::copyAccounts(std::vector<B2JConfig::Account> &buffer) {
  fImpl->fAccounts.swap(buffer);
}

void B2JConfig::onClickImportAccountFromLauncherButton() {
  if (fImportAccountWorker) {
    return;
  }
  if (fImportAccountFromLauncher->getToggleState()) {
    if (fAccounts.empty()) {
      fImportAccountFromLauncher->setEnabled(false);
      stopTimer();
      fStartButton->setEnabled(false);
      fBackButton->setEnabled(false);
      fImportAccountWorker.reset(new ImportAccountWorker(this));
      fImportAccountWorker->startThread();
    } else {
      fAccountList->setEnabled(true);
    }
  } else {
    fAccountList->setEnabled(false);
  }
}

void B2JConfig::handleAsyncUpdate() {
  if (fImportAccountWorker) {
    fImportAccountWorker->copyAccounts(fAccounts);
  }
  fImportAccountWorker.reset();
  fAccountList->clear();
  for (int i = 0; i < fAccounts.size(); i++) {
    Account account = fAccounts[i];
    fAccountList->addItem(account.toString(), i + 1);
  }
  if (fAccounts.size() >= 1) {
    fAccountList->setSelectedId(1);
  }
  fAccountList->setEnabled(true);
  fImportAccountFromLauncher->setEnabled(true);
  fStartButton->setEnabled(true);
  fBackButton->setEnabled(true);
}

} // namespace je2be::gui::component::b2j
