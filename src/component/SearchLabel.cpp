#include "SearchLabel.h"

using namespace juce;

namespace je2be::desktop::component {

static void copyColourIfSpecified(Label &l, TextEditor &ed, int colourID, int targetColourID) {
  if (l.isColourSpecified(colourID) || l.getLookAndFeel().isColourSpecified(colourID))
    ed.setColour(targetColourID, l.findColour(colourID));
}

juce::TextEditor *SearchLabel::createEditorComponent() {
  auto *ed = new TextEditor(getName());
  ed->applyFontToAllText(getLookAndFeel().getLabelFont(*this));
  copyAllExplicitColoursTo(*ed);

  copyColourIfSpecified(*this, *ed, textWhenEditingColourId, TextEditor::textColourId);
  copyColourIfSpecified(*this, *ed, backgroundWhenEditingColourId, TextEditor::backgroundColourId);
  copyColourIfSpecified(*this, *ed, outlineWhenEditingColourId, TextEditor::focusedOutlineColourId);

  ed->onTextChange = [this]() {
    if (!onTextUpdate) {
      return;
    }
    onTextUpdate();
  };

  return ed;
}

String SearchLabel::getCurrentText() {
  TextEditor *e = getCurrentTextEditor();
  if (e) {
    return e->getText();
  } else {
    return getText();
  }
}

} // namespace je2be::desktop::component
