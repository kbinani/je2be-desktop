#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace je2be::gui::component {

class TextButton;

class SelectInputType : public juce::Component {
public:
  explicit SelectInputType();
  ~SelectInputType() override;

  void paint(juce::Graphics &) override;

  void onAboutButtonClicked();
  void onConvertJavaClicked();
  void onConvertBedrockClicked();

private:
  std::unique_ptr<juce::Label> fLabel;
  std::unique_ptr<TextButton> fConvertJava;
  std::unique_ptr<TextButton> fConvertBedrock;
  std::unique_ptr<TextButton> fAboutButton;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SelectInputType)
};

} // namespace je2be::gui::component
