#include "LocalizationHelper.h"
#include "BinaryData.h"

#if JUCE_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

using namespace juce;

LocalisedStrings *LocalizationHelper::CurrentLocalisedStrings() {
  LANGID lang = GetSystemDefaultUILanguage();
  if (lang == 0x0411) {
    return Japanese();
  }
  return nullptr;
}

LocalisedStrings *LocalizationHelper::Japanese() {
  return LoadLocalisedStrings(BinaryData::japanese_lang,
                              BinaryData::japanese_langSize);
}
