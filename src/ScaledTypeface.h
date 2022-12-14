#pragma once

#include "LocalizationHelper.h"
#include <juce_gui_extra/juce_gui_extra.h>

namespace je2be::desktop {

class ScaledTypeface : public juce::CustomTypeface {
  enum FontFamily : int {
    Neutral,
    Japanese,
    TraditionalChinese,
    SimplifiedChinese,
    Emoji,

    kNumFontFamilies,
  };

public:
  explicit ScaledTypeface(float scale) : fScale(scale) {
    fOrder[0] = Neutral;

    std::vector<FontFamily> remaining = {Japanese, SimplifiedChinese, TraditionalChinese};
    jassert(remaining.size() == kNumFontFamilies - 2);

    auto languages = LocalizationHelper::PreferredLanguages();
    int idx = 1;
    for (int i = 0; i < languages.size() && idx < kNumFontFamilies; i++) {
      juce::String lang = languages[i];
      if (lang == "ja-JP") {
        if (auto found = std::find(remaining.begin(), remaining.end(), Japanese); found != remaining.end()) {
          fOrder[idx] = Japanese;
          remaining.erase(found);
          idx++;
        }
      } else if (lang.startsWith("zh-Hans")) {
        if (auto found = std::find(remaining.begin(), remaining.end(), SimplifiedChinese); found != remaining.end()) {
          fOrder[idx] = SimplifiedChinese;
          remaining.erase(found);
          idx++;
        }
      } else if (lang.startsWith("zh-Hant")) {
        if (auto found = std::find(remaining.begin(), remaining.end(), TraditionalChinese); found != remaining.end()) {
          fOrder[idx] = TraditionalChinese;
          remaining.erase(found);
          idx++;
        }
      }
    }
    while (!remaining.empty()) {
      fOrder[idx] = remaining[0];
      idx++;
      remaining.erase(remaining.begin());
    }
    jassert(idx == kNumFontFamilies - 1);
    fOrder[idx] = Emoji;
    for (int i = 0; i < kNumFontFamilies; i++) {
      fFontFamilies[i] = nullptr;
    }
    setCharacteristics("scaledtypeface", "Regular", 0.801104963f, L' ');
  }

  bool loadGlyphIfPossible(juce::juce_wchar c) override {
    using namespace juce;
    for (int i = 0; i < kNumFontFamilies; i++) {
      Typeface *typeface = ensureTypeface(fOrder[i]);
      if (!typeface) {
        continue;
      }

      Array<int> glyphIndexes;
      Array<float> offsets;
      typeface->getGlyphPositions(String::charToString(c), glyphIndexes, offsets);

      const int glyphIndex = glyphIndexes.getFirst();
      if (glyphIndex == 65535) {
        continue;
      }

      if (glyphIndex >= 0 && glyphIndexes.size() > 0) {
        auto glyphWidth = offsets[1];

        Path p;
        typeface->getOutlineForGlyph(glyphIndex, p);

        addGlyph(c, p, glyphWidth);
        return true;
      }
    }

    return false;
  }

  float getStringWidth(juce::String const &text) override {
    return juce::CustomTypeface::getStringWidth(text) * fScale;
  }

  void getGlyphPositions(juce::String const &text, juce::Array<int> &glyphs, juce::Array<float> &xOffsets) override {
    using namespace juce;
    CustomTypeface::getGlyphPositions(text, glyphs, xOffsets);
    for (int i = 0; i < xOffsets.size(); i++) {
      xOffsets.set(i, xOffsets[i] * fScale);
    }
  }

  bool getOutlineForGlyph(int glyphNumber, juce::Path &path) override {
    juce::Path p;
    bool ok = juce::CustomTypeface::getOutlineForGlyph(glyphNumber, p);
    if (!ok) {
      return false;
    }
    p.applyTransform(juce::AffineTransform::scale(fScale, fScale));
    path = p;
    return ok;
  }

  juce::EdgeTable *getEdgeTableForGlyph(int glyphNumber, juce::AffineTransform const &transform, float fontHeight) override {
    using namespace juce;
    AffineTransform t = transform.scaled(fScale, fScale);
    return CustomTypeface::getEdgeTableForGlyph(glyphNumber, t, fontHeight);
  }

private:
  juce::Typeface *ensureTypeface(FontFamily fontFamily) {
    using namespace juce;
    if (!fFontFamilies[fontFamily]) {
      switch (fontFamily) {
      case Neutral:
        fFontFamilies[fontFamily] = Typeface::createSystemTypefaceFor(BinaryData::NotoSansRegular_ttf, BinaryData::NotoSansRegular_ttfSize);
        break;
      case Japanese:
        fFontFamilies[fontFamily] = Typeface::createSystemTypefaceFor(BinaryData::NotoSansJPRegular_otf, BinaryData::NotoSansJPRegular_otfSize);
        break;
      case Emoji:
        fFontFamilies[fontFamily] = Typeface::createSystemTypefaceFor(BinaryData::NotoColorEmojiRegular_ttf, BinaryData::NotoColorEmojiRegular_ttfSize);
        break;
      case TraditionalChinese:
        fFontFamilies[fontFamily] = Typeface::createSystemTypefaceFor(BinaryData::NotoSansTCRegular_otf, BinaryData::NotoSansTCRegular_otfSize);
        break;
      case SimplifiedChinese:
        fFontFamilies[fontFamily] = Typeface::createSystemTypefaceFor(BinaryData::NotoSansSCRegular_otf, BinaryData::NotoSansSCRegular_otfSize);
        break;
      }
    }
    return fFontFamilies[fontFamily].get();
  }

private:
  float const fScale;
  juce::Typeface::Ptr fFontFamilies[kNumFontFamilies];
  FontFamily fOrder[kNumFontFamilies];
};

} // namespace je2be::desktop
