#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace je2be::gui::component {

class TextButton;

class ModeSelect : public juce::Component {
public:
  explicit ModeSelect();
  ~ModeSelect();

  void paint(juce::Graphics &) override;

  void onAboutButtonClicked();
  void onJavaToBedrockButtonClicked();
  void onBedrockToJavaButtonClicked();
  void onXbox360ToJavaButtonClicked();
  void onXbox360ToBedrockButtonClicked();

private:
  std::unique_ptr<juce::Label> fLabel;
  std::unique_ptr<TextButton> fJavaToBedrockButton;
  std::unique_ptr<TextButton> fBedrockToJavaButton;
  std::unique_ptr<TextButton> fXbox360ToJavaButton;
  std::unique_ptr<TextButton> fXbox360ToBedrockButton;
  std::unique_ptr<TextButton> fAboutButton;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModeSelect)
};

} // namespace je2be::gui::component
