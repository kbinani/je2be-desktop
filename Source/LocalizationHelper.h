#pragma once

#include <JuceHeader.h>

class LocalizationHelper
{
public:
    static LocalisedStrings *CurrentLocalisedStrings();

private:
    static LocalisedStrings *LoadLocalisedStrings(char const* data, int size)
    {
        std::vector<char> d(size + 1);
        std::copy_n(data, size, d.begin());
        String t = String::fromUTF8(d.data());
        return new LocalisedStrings(t, false);
    }

    static LocalisedStrings *Japanese();

    LocalizationHelper() = delete;
};
