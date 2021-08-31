#pragma once

#include "ChooseInputComponent.h"
#include <juce_gui_extra/juce_gui_extra.h>

using namespace juce;

class MainWindow : public juce::DocumentWindow {
public:
  MainWindow(juce::String name)
      : DocumentWindow(
            name,
            juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(
                juce::ResizableWindow::backgroundColourId),
            DocumentWindow::closeButton | DocumentWindow::minimiseButton) {
    setUsingNativeTitleBar(true);
    setContentOwned(new ChooseInputComponent(std::nullopt), true);

#if JUCE_IOS || JUCE_ANDROID
    setFullScreen(true);
#else
    setResizable(false, false);
    centreWithSize(getWidth(), getHeight());
#endif

    setVisible(true);
  }

  void closeButtonPressed() override {
    sFileChooser.reset();
    JUCEApplication::getInstance()->systemRequestedQuit();
  }

  static std::unique_ptr<juce::FileChooser> sFileChooser;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};
