#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace je2be::desktop {

class LocalizationHelper {
public:
  static juce::LocalisedStrings *CurrentLocalisedStrings();
  static std::vector<juce::String> PreferredLanguages();

private:
  static juce::LocalisedStrings *LoadLocalisedStrings(char const *data, int size) {
    std::vector<char> d(size + 1);
    std::copy_n(data, size, d.begin());
    juce::String t = juce::String::fromUTF8(d.data());
    return new juce::LocalisedStrings(t, false);
  }

  static juce::LocalisedStrings *Language_ja_JP();
  static juce::LocalisedStrings *Language_zh_Hans();

  LocalizationHelper() = delete;
};

} // namespace je2be::desktop
