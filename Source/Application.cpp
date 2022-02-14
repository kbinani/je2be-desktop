#include "CommandID.h"
#include "Constants.h"
#include "J2BChooseInputComponent.h"
#include "J2BChooseOutputComponent.h"
#include "J2BConfigComponent.h"
#include "J2BConvertProgressComponent.h"
#include "J2BCopyProgressComponent.h"
#include "LocalizationHelper.h"
#include "LookAndFeel.h"
#include "MainWindow.h"
#include "TemporaryDirectory.h"

using namespace juce;

namespace je2be::gui {

class Application : public juce::JUCEApplication {
public:
  Application() {}

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
    fLaf.reset(new je2be::gui::LookAndFeel);
    LookAndFeel::setDefaultLookAndFeel(fLaf.get());

    LocalisedStrings::setCurrentMappings(LocalizationHelper::CurrentLocalisedStrings());

    TemporaryDirectory::CleanupAsync();

    fMainWindow.reset(new MainWindow(getApplicationName()));
  }

  void shutdown() override {
    fMainWindow = nullptr;
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
    Component *current = fMainWindow->getContentComponent();
    switch (info.commandID) {
    case gui::toConfig: {
      auto provider = dynamic_cast<J2BChooseInputStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto config = new ConfigComponent(provider->getChooseInputState());
      fMainWindow->setContentOwned(config, true);
      return true;
    }
    case gui::toChooseInput: {
      std::optional<J2BChooseInputState> state;
      auto provider = dynamic_cast<J2BChooseInputStateProvider *>(current);
      if (provider) {
        state = provider->getChooseInputState();
      }
      auto chooseInput = new ChooseInputComponent(state);
      fMainWindow->setContentOwned(chooseInput, true);
      return true;
    }
    case gui::toConvert: {
      auto provider = dynamic_cast<J2BConfigStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto convert = new ConvertProgressComponent(provider->getConfigState());
      fMainWindow->setContentOwned(convert, true);
      return true;
    }
    case gui::toChooseOutput: {
      auto provider = dynamic_cast<J2BConvertStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto chooseOutput = new ChooseOutputComponent(provider->getConvertState());
      fMainWindow->setContentOwned(chooseOutput, true);
      return true;
    }
    case gui::toCopy: {
      auto provider = dynamic_cast<J2BChooseOutputStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto copy = new CopyProgressComponent(provider->getChooseOutputState());
      fMainWindow->setContentOwned(copy, true);
      return true;
    }
    default:
      return JUCEApplication::perform(info);
    }
  }

private:
  std::unique_ptr<MainWindow> fMainWindow;
  SharedResourcePointer<TooltipWindow> fTooltipWindow;
  std::unique_ptr<je2be::gui::LookAndFeel> fLaf;
};

} // namespace je2be::gui

START_JUCE_APPLICATION(je2be::gui::Application)
