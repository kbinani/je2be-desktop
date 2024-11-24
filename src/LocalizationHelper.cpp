#include "LocalizationHelper.h"
#include "BinaryData.h"

#if JUCE_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

using namespace juce;

namespace je2be::desktop {

LocalisedStrings *LocalizationHelper::CurrentLocalisedStrings() {
  auto languages = PreferredLanguages();
  if (languages.empty()) {
    return nullptr;
  }
  auto language = languages[0];
  if (language == "ja-JP") {
    return Language_ja_JP();
  } else if (language == "zh-Hans") {
    return Language_zh_Hans();
  }
  return nullptr;
}

std::vector<String> LocalizationHelper::PreferredLanguages() {
  using namespace std;
  vector<String> ret;

  ULONG num = 0;
  ULONG size = 0;
  if (!GetSystemPreferredUILanguages(MUI_LANGUAGE_NAME, &num, nullptr, &size)) {
    return ret;
  }
  wstring buffer;
  buffer.resize(size, (WCHAR)0);
  num = 0;
  size = buffer.size();
  if (!GetSystemPreferredUILanguages(MUI_LANGUAGE_NAME, &num, buffer.data(), &size)) {
    return ret;
  }
  int off = 0;
  while (off < buffer.size()) {
    size_t idx = buffer.find(L'\0', off);
    if (idx == wstring::npos) {
      break;
    }
    String lang(buffer.substr(off, idx).c_str());
    if (lang.isNotEmpty()) {
      ret.push_back(lang);
    }
    off = idx + 1;
  }
  String lang(buffer.substr(off).c_str());
  if (lang.isNotEmpty()) {
    ret.push_back(lang);
  }
  return ret;
}

LocalisedStrings *LocalizationHelper::Language_ja_JP() {
  return LoadLocalisedStrings(BinaryData::jaJP_lang,
                              BinaryData::jaJP_langSize);
}

LocalisedStrings *LocalizationHelper::Language_zh_Hans() {
  return LoadLocalisedStrings(BinaryData::zhHans_lang,
                              BinaryData::zhHans_langSize);
}

} // namespace je2be::desktop
