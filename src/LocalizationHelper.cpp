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
  using namespace std;
  ULONG num = 0;
  ULONG size = 0;
  if (!GetSystemPreferredUILanguages(MUI_LANGUAGE_NAME, &num, nullptr, &size)) {
    return nullptr;
  }
  wstring buffer;
  buffer.resize(size, (WCHAR)0);
  num = 0;
  size = buffer.size();
  if (!GetSystemPreferredUILanguages(MUI_LANGUAGE_NAME, &num, buffer.data(), &size)) {
    return nullptr;
  }
  String preferred(buffer.c_str());
  if (preferred == "ja-JP") {
    return Japanese();
  }
  return nullptr;
}

LocalisedStrings *LocalizationHelper::Japanese() {
  return LoadLocalisedStrings(BinaryData::japanese_lang,
                              BinaryData::japanese_langSize);
}

} // namespace je2be::desktop
