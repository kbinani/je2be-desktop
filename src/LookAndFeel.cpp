#include "LookAndFeel.h"
#include "BinaryData.h"
#include "ScaledTypeface.h"

using namespace juce;

namespace je2be::desktop {

LookAndFeel::LookAndFeel() {
  MemoryInputStream mem(BinaryData::font_bin, BinaryData::font_binSize, false);
  auto ct = new ScaledTypeface(new CustomTypeface(mem), 1.15f);
  fTypeface = ReferenceCountedObjectPtr<Typeface>(ct);
}

Typeface::Ptr LookAndFeel::getTypefaceForFont(Font const &font) {
  return fTypeface;
}

} // namespace je2be::desktop
