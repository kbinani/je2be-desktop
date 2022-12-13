#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace je2be::desktop {

class ScaledTypeface : public juce::Typeface {
public:
  explicit ScaledTypeface(juce::Typeface *base, float scale) : juce::Typeface(base->getName(), base->getStyle()), fBase(base), fScale(scale) {}

  float getAscent() const override {
    return fBase->getAscent();
  }

  float getDescent() const override {
    return fBase->getDescent();
  }

  float getHeightToPointsFactor() const override {
    return fBase->getHeightToPointsFactor();
  }

  float getStringWidth(juce::String const &text) override {
    return fBase->getStringWidth(text) * fScale;
  }

  void getGlyphPositions(juce::String const &text, juce::Array<int> &glyphs, juce::Array<float> &xOffsets) override {
    fBase->getGlyphPositions(text, glyphs, xOffsets);
    for (int i = 0; i < xOffsets.size(); i++) {
      xOffsets.set(i, xOffsets[i] * fScale);
    }
  }

  bool getOutlineForGlyph(int glyphNumber, juce::Path &path) override {
    bool ok = fBase->getOutlineForGlyph(glyphNumber, path);
    path.applyTransform(juce::AffineTransform::scale(fScale, fScale));
    return ok;
  }

private:
  std::unique_ptr<juce::Typeface> fBase;
  float fScale;
};

} // namespace je2be::desktop
