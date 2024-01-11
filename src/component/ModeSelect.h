#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace je2be::desktop::component {

class TextButton;

class ModeSelect : public juce::Component {
  enum class From {
    Java,
    Bedrock,
    Xbox360,
    PS3,
  };
  enum class To {
    Java,
    Bedrock,
  };

public:
  explicit ModeSelect();
  ~ModeSelect();

  void paint(juce::Graphics &) override;

  void onAboutButtonClicked();
  void onNextButtonClicked();

private:
  void update();

private:
  std::unique_ptr<juce::Label> fLabel;
  std::unique_ptr<TextButton> fFromJavaButton;
  std::unique_ptr<TextButton> fFromBedrockButton;
  std::unique_ptr<TextButton> fFromXbox360Button;
  std::unique_ptr<TextButton> fFromPS3Button;
  std::unique_ptr<TextButton> fToJavaButton;
  std::unique_ptr<TextButton> fToBedrockButton;
  std::unique_ptr<TextButton> fAboutButton;
  std::unique_ptr<TextButton> fNextButton;
  juce::Path fJavaToBedrockPath;
  juce::Path fBedrockToJavaPath;
  juce::Path fXbox360ToJavaPath;
  juce::Path fXbox360ToBedrockPath;
  juce::Path fPS3ToJavaPath;
  juce::Path fPS3ToBedrockPath;
  juce::Path fArrowHeadBedrockToJava;
  juce::Path fArrowHeadJavaToBedrock;
  juce::Path fArrowHeadXbox360ToJava;
  juce::Path fArrowHeadXbox360ToBedrock;
  juce::Path fArrowHeadPS3ToJava;
  juce::Path fArrowHeadPS3ToBedrock;
  std::optional<From> fFrom;
  std::optional<To> fTo;
  std::optional<From> fLastFrom;
  std::optional<To> fLastTo;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModeSelect)
};

} // namespace je2be::desktop::component
