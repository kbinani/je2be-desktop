#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace je2be::desktop::component {

class SearchLabel : public juce::Label {
public:
  juce::TextEditor *createEditorComponent() override;
  std::function<void()> onTextUpdate;
  juce::String getCurrentText();
};

} // namespace je2be::desktop::component
