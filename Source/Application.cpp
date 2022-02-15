#include "B2JChooseInputComponent.h"
#include "B2JConfigComponent.h"
#include "B2JConvertProgressComponent.h"
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
    commands.addArray({gui::toJ2BConfig, gui::toJ2BChooseInput, gui::toJ2BConvert, gui::toJ2BChooseOutput, gui::toJ2BCopy, gui::toModeSelect, gui::toB2JChooseInput, gui::toB2JConfig, gui::toB2JConvert});
  }

  void getCommandInfo(CommandID commandID, ApplicationCommandInfo &result) override {
    result.setInfo("", "", "", ApplicationCommandInfo::CommandFlags::hiddenFromKeyEditor);
  }

  bool perform(InvocationInfo const &info) override {
    Component *current = fMainWindow->getContentComponent();
    switch (info.commandID) {
    case gui::toJ2BConfig: {
      auto provider = dynamic_cast<J2BChooseInputStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto config = new J2BConfigComponent(provider->getChooseInputState());
      fMainWindow->setContentOwned(config, true);
      return true;
    }
    case gui::toJ2BChooseInput: {
      std::optional<J2BChooseInputState> state;
      auto provider = dynamic_cast<J2BChooseInputStateProvider *>(current);
      if (provider) {
        state = provider->getChooseInputState();
      }
      auto chooseInput = new J2BChooseInputComponent(state);
      fMainWindow->setContentOwned(chooseInput, true);
      return true;
    }
    case gui::toJ2BConvert: {
      auto provider = dynamic_cast<J2BConfigStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto convert = new J2BConvertProgressComponent(provider->getConfigState());
      fMainWindow->setContentOwned(convert, true);
      return true;
    }
    case gui::toJ2BChooseOutput: {
      auto provider = dynamic_cast<J2BConvertStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto chooseOutput = new J2BChooseOutputComponent(provider->getConvertState());
      fMainWindow->setContentOwned(chooseOutput, true);
      return true;
    }
    case gui::toJ2BCopy: {
      auto provider = dynamic_cast<J2BChooseOutputStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto copy = new J2BCopyProgressComponent(provider->getChooseOutputState());
      fMainWindow->setContentOwned(copy, true);
      return true;
    }
    case gui::toModeSelect: {
      auto modeSelect = new ModeSelectComponent;
      fMainWindow->setContentOwned(modeSelect, true);
      return true;
    }
    case gui::toB2JChooseInput: {
      std::optional<B2JChooseInputState> state;
      auto provider = dynamic_cast<B2JChooseInputStateProvider *>(current);
      if (provider) {
        state = provider->getChooseInputState();
      }
      auto chooseInput = new B2JChooseInputComponent(state);
      fMainWindow->setContentOwned(chooseInput, true);
      return true;
    }
    case gui::toB2JConfig: {
      auto provider = dynamic_cast<B2JChooseInputStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto config = new B2JConfigComponent(provider->getChooseInputState());
      fMainWindow->setContentOwned(config, true);
      return true;
    }
    case gui::toB2JConvert: {
      auto provider = dynamic_cast<B2JConfigStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto convert = new B2JConvertProgressComponent(provider->getConfigState());
      fMainWindow->setContentOwned(convert, true);
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
