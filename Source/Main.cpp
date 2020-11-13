#include "ChooseInputComponent.h"
#include "ChooseOutputComponent.h"
#include "CommandID.h"
#include "ConfigComponent.h"
#include "Constants.h"
#include "ConvertProgressComponent.h"
#include "CopyProgressComponent.h"
#include "LocalizationHelper.h"
#include <JuceHeader.h>

class je2beApplication : public juce::JUCEApplication {
public:
  je2beApplication() {}

  const juce::String getApplicationName() override {
    return ProjectInfo::projectName;
  }
  const juce::String getApplicationVersion() override {
    return ProjectInfo::versionString;
  }
  bool moreThanOneInstanceAllowed() override { return true; }

  void initialise(const juce::String &commandLine) override {
    String typeFaceName = "Meiryo UI";
    Desktop::getInstance()
        .getDefaultLookAndFeel()
        .setDefaultSansSerifTypefaceName(typeFaceName);

    LocalisedStrings::setCurrentMappings(
        LocalizationHelper::CurrentLocalisedStrings());

    mainWindow.reset(new MainWindow(getApplicationName()));
  }

  void shutdown() override { mainWindow = nullptr; }

  void systemRequestedQuit() override { quit(); }

  bool perform(InvocationInfo const &info) override {
    Component *current = mainWindow->getContentComponent();
    switch (info.commandID) {
    case gui::toConfig: {
      auto provider = dynamic_cast<ChooseInputStateProvider *>(current);
      if (!provider)
        return false;
      auto config = new ConfigComponent(provider->getChooseInputState());
      mainWindow->setContentOwned(config, true);
      return true;
    }
    case gui::toChooseInput: {
      auto chooseInput = new ChooseInputComponent();
      mainWindow->setContentOwned(chooseInput, true);
      return true;
    }
    case gui::toConvert: {
      auto provider = dynamic_cast<ConfigStateProvider *>(current);
      if (!provider)
        return false;
      auto convert = new ConvertProgressComponent(provider->getConfigState());
      mainWindow->setContentOwned(convert, true);
      return true;
    }
    case gui::toChooseOutput: {
      auto provider = dynamic_cast<ConvertStateProvider *>(current);
      if (!provider)
        return false;
      auto chooseOutput =
          new ChooseOutputComponent(provider->getConvertState());
      mainWindow->setContentOwned(chooseOutput, true);
      return true;
    }
    case gui::toCopy: {
      auto provider = dynamic_cast<ChooseOutputStateProvider *>(current);
      if (!provider)
        return false;
      auto copy = new CopyProgressComponent(provider->getChooseOutputState());
      mainWindow->setContentOwned(copy, true);
      return true;
    }
    default:
      return JUCEApplication::perform(info);
    }
  }

  class MainWindow : public juce::DocumentWindow {
  public:
    MainWindow(juce::String name)
        : DocumentWindow(
              name,
              juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(
                  juce::ResizableWindow::backgroundColourId),
              DocumentWindow::closeButton | DocumentWindow::minimiseButton) {
      setUsingNativeTitleBar(true);
      setContentOwned(new ChooseInputComponent(), true);

#if JUCE_IOS || JUCE_ANDROID
      setFullScreen(true);
#else
      setResizable(false, false);
      centreWithSize(getWidth(), getHeight());
#endif

      setVisible(true);
    }

    void closeButtonPressed() override {
      JUCEApplication::getInstance()->systemRequestedQuit();
    }

  private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
  };

private:
  std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(je2beApplication)
