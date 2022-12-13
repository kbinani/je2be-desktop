#include <juce_gui_extra/juce_gui_extra.h>
#include <vector>

using namespace juce;
using namespace std;

// Noto_Sans/NotoSans-Regular.ttf
// Noto_Sans_JP/NotoSansJP-Regular.otf

int main(int argc, char *argv[]) {
  auto ct = new CustomTypeface();
  for (int i = 1; i < argc; i++) {
    char *file = argv[i];
    File f(file);
    FileInputStream stream(f);
    vector<uint8_t> buffer(stream.getTotalLength());
    stream.read(buffer.data(), buffer.size());
    auto typeface = Typeface::createSystemTypefaceFor(buffer.data(), buffer.size());
    ct->addGlyphsFromOtherTypeface(*typeface, 0, 65536);
  }
  File out("out.bin");
  auto stream = out.createOutputStream();
  ct->writeToStream(*stream);
}
