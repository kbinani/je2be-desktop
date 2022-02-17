#pragma once

#include "ModeSelectComponent.h"
#include <juce_gui_extra/juce_gui_extra.h>

using namespace juce;

namespace je2be::gui {

class MainWindow : public juce::DocumentWindow {
public:
  MainWindow(juce::String name)
      : DocumentWindow(
            name,
            juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(
                juce::ResizableWindow::backgroundColourId),
            DocumentWindow::closeButton | DocumentWindow::minimiseButton) {
    setUsingNativeTitleBar(true);
    setContentOwned(new ModeSelectComponent, true);

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
    sAsyncTasks.reset();
    JUCEApplication::getInstance()->systemRequestedQuit();
  }

  static void QueueDeletingDirectory(File directory);

  static std::unique_ptr<juce::FileChooser> sFileChooser;
  static std::unique_ptr<juce::ThreadPool> sAsyncTasks;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

} // namespace je2be::gui
