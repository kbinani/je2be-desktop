#include "ChooseInputComponent.h"
#include "ChooseOutputComponent.h"
#include "CommandID.h"
#include "ConfigComponent.h"
#include "Constants.h"
#include "ConvertProgressComponent.h"
#include "CopyProgressComponent.h"
#include "LocalizationHelper.h"
#include "MainWindow.h"
#include "TemporaryDirectory.h"

using namespace juce;

class je2beApplication : public juce::JUCEApplication {
public:
  je2beApplication() {}

  const juce::String getApplicationName() override {
    return JUCE_APPLICATION_NAME_STRING;
  }

  const juce::String getApplicationVersion() override {
    return JUCE_APPLICATION_VERSION_STRING;
  }

  bool moreThanOneInstanceAllowed() override {
    return true;
  }

  void initialise(const juce::String &commandLine) override {
    String typeFaceName = "Meiryo UI";
    Desktop::getInstance().getDefaultLookAndFeel().setDefaultSansSerifTypefaceName(typeFaceName);

    LocalisedStrings::setCurrentMappings(LocalizationHelper::CurrentLocalisedStrings());

    TemporaryDirectory::CleanupAsync();

    mainWindow.reset(new MainWindow(getApplicationName()));
  }

  void shutdown() override {
    mainWindow = nullptr;
  }

  void systemRequestedQuit() override {
    quit();
  }

  void getAllCommands(Array<CommandID> &commands) override {
    JUCEApplication::getAllCommands(commands);
    commands.addArray({gui::toConfig, gui::toChooseInput, gui::toConvert, gui::toChooseOutput, gui::toCopy});
  }

  void getCommandInfo(CommandID commandID, ApplicationCommandInfo &result) override {
    result.setInfo("", "", "", ApplicationCommandInfo::CommandFlags::hiddenFromKeyEditor);
  }

  bool perform(InvocationInfo const &info) override {
    Component *current = mainWindow->getContentComponent();
    switch (info.commandID) {
    case gui::toConfig: {
      auto provider = dynamic_cast<ChooseInputStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto config = new ConfigComponent(provider->getChooseInputState());
      mainWindow->setContentOwned(config, true);
      return true;
    }
    case gui::toChooseInput: {
      std::optional<ChooseInputState> state;
      auto provider = dynamic_cast<ChooseInputStateProvider *>(current);
      if (provider) {
        state = provider->getChooseInputState();
      }
      auto chooseInput = new ChooseInputComponent(state);
      mainWindow->setContentOwned(chooseInput, true);
      return true;
    }
    case gui::toConvert: {
      auto provider = dynamic_cast<ConfigStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto convert = new ConvertProgressComponent(provider->getConfigState());
      mainWindow->setContentOwned(convert, true);
      return true;
    }
    case gui::toChooseOutput: {
      auto provider = dynamic_cast<ConvertStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto chooseOutput = new ChooseOutputComponent(provider->getConvertState());
      mainWindow->setContentOwned(chooseOutput, true);
      return true;
    }
    case gui::toCopy: {
      auto provider = dynamic_cast<ChooseOutputStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto copy = new CopyProgressComponent(provider->getChooseOutputState());
      mainWindow->setContentOwned(copy, true);
      return true;
    }
    default:
      return JUCEApplication::perform(info);
    }
  }

private:
  std::unique_ptr<MainWindow> mainWindow;
  SharedResourcePointer<TooltipWindow> tooltipWindow;
};

START_JUCE_APPLICATION(je2beApplication)
