#pragma once

#include "component/ModeSelect.h"

namespace je2be::desktop::component {

class MainWindow : public juce::DocumentWindow {
public:
  explicit MainWindow(juce::String name)
      : juce::DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), juce::DocumentWindow::closeButton | juce::DocumentWindow::minimiseButton) {
    setUsingNativeTitleBar(true);
    setContentOwned(new ModeSelect, true);

#if JUCE_IOS || JUCE_ANDROID
    setFullScreen(true);
#else
    setResizable(false, false);
    centreWithSize(getWidth(), getHeight());
#endif
  }

  void closeButtonPressed() override {
    sFileChooser.reset();
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
  }

  static std::unique_ptr<juce::FileChooser> sFileChooser;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

} // namespace je2be::desktop::component
