#include "LookAndFeel.h"
#include "BinaryData.h"
#include "ScaledTypeface.h"

using namespace juce;

namespace je2be::desktop {

LookAndFeel::LookAndFeel() {
  auto ct = new ScaledTypeface(1.15f);
  fTypeface = ReferenceCountedObjectPtr<Typeface>(ct);
  setDefaultSansSerifTypeface(fTypeface);
}

} // namespace je2be::desktop
