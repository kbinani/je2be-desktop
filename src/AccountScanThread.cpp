#include <nlohmann/json.hpp>

#include "AccountScanThread.h"
#include "GameDirectory.h"

using namespace juce;

namespace je2be::desktop {

class AccountScanThread::Impl {
public:
  explicit Impl(AsyncUpdater *parent) : fParent(parent) {}

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

    vector<File> jsonFiles;
    jsonFiles.push_back(saves.getParentDirectory().getChildFile("launcher_accounts.json"));
    jsonFiles.push_back(saves.getParentDirectory().getChildFile("launcher_accounts_microsoft_store.json"));

    jsonFiles.erase(remove_if(jsonFiles.begin(), jsonFiles.end(), [](File const &file) {
                      return !file.existsAsFile();
                    }),
                    jsonFiles.end());

    sort(jsonFiles.begin(), jsonFiles.end(), [](File const &a, File const &b) {
      return a.getLastModificationTime().toMilliseconds() < b.getLastModificationTime().toMilliseconds();
    });

    unordered_map<string, Account> collected;
    string activeAccountLocalId;

    for (File jsonFile : jsonFiles) {
      juce::String jsonString = jsonFile.loadFileAsString();
      string jsonStdString(jsonString.toUTF8(), jsonString.getNumBytesAsUTF8());
      auto json = nlohmann::json::parse(jsonStdString);
      auto accounts = json["accounts"];
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
      try {
        activeAccountLocalId = json["activeAccountLocalId"].get<string>();
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

  AsyncUpdater *const fParent;
  std::vector<Account> fAccounts;
};

AccountScanThread::AccountScanThread(AsyncUpdater *parent) : Thread("je2be::desktop::AccountScanThread"), fImpl(new Impl(parent)) {}

AccountScanThread::~AccountScanThread() {}

void AccountScanThread::run() {
  fImpl->run();
}

void AccountScanThread::copyAccounts(std::vector<Account> &buffer) {
  fImpl->fAccounts.swap(buffer);
}

} // namespace je2be::desktop
