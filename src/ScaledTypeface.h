#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace je2be::desktop {

class ScaledTypeface : public juce::CustomTypeface {
  enum Region : int {
    Neutral,
    Japanese,
    Emoji,

    kMaxRegion,
  };

public:
  explicit ScaledTypeface(float scale) : fScale(scale) {
    fOrder[0] = Neutral;
    fOrder[1] = Japanese;
    fOrder[2] = Emoji;
    for (int i = 0; i < kMaxRegion; i++) {
      fFaces[i] = nullptr;
    }
    setCharacteristics("scaledtypeface", "Regular", 0.801104963f, L' ');
  }

  bool loadGlyphIfPossible(juce::juce_wchar c) override {
    using namespace juce;
    for (int i = 0; i < kMaxRegion; i++) {
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
  juce::Typeface *ensureTypeface(Region region) {
    using namespace juce;
    if (!fFaces[region]) {
      switch (region) {
      case Neutral:
        fFaces[region] = Typeface::createSystemTypefaceFor(BinaryData::NotoSansRegular_ttf, BinaryData::NotoSansRegular_ttfSize);
        break;
      case Japanese: {
        fFaces[region] = Typeface::createSystemTypefaceFor(BinaryData::NotoSansJPRegular_otf, BinaryData::NotoSansJPRegular_otfSize);
        break;
      }
      case Emoji:
        fFaces[region] = Typeface::createSystemTypefaceFor(BinaryData::NotoColorEmojiRegular_ttf, BinaryData::NotoColorEmojiRegular_ttfSize);
        break;
      }
    }
    return fFaces[region].get();
  }

private:
  float const fScale;
  juce::Typeface::Ptr fFaces[kMaxRegion];
  Region fOrder[kMaxRegion];
};

} // namespace je2be::desktop
