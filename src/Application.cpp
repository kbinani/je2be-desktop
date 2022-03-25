#include "CommandID.h"
#include "Constants.h"
#include "LocalizationHelper.h"
#include "LookAndFeel.h"
#include "TemporaryDirectory.h"
#include "component/ChooseBedrockInput.h"
#include "component/ChooseBedrockOutput.h"
#include "component/ChooseJavaInput.h"
#include "component/ChooseJavaOutput.h"
#include "component/CopyBedrockArtifactProgress.h"
#include "component/CopyJavaArtifactProgress.h"
#include "component/MainWindow.h"
#include "component/b2j/B2JConfig.h"
#include "component/b2j/B2JConvertProgress.h"
#include "component/j2b/J2BConfig.h"
#include "component/j2b/J2BConvertProgress.h"

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

    fMainWindow.reset(new component::MainWindow(getApplicationName()));
  }

  void shutdown() override {
    fMainWindow = nullptr;
  }

  void systemRequestedQuit() override {
    quit();
  }

  void getAllCommands(Array<CommandID> &commands) override {
    JUCEApplication::getAllCommands(commands);
    commands.addArray({gui::toJ2BConfig, gui::toJ2BChooseInput, gui::toJ2BConvert, gui::toJ2BChooseOutput, gui::toJ2BCopy, gui::toModeSelect, gui::toB2JChooseInput, gui::toB2JConfig, gui::toB2JConvert, gui::toB2JChooseOutput, gui::toB2JCopy});
  }

  void getCommandInfo(CommandID commandID, ApplicationCommandInfo &result) override {
    result.setInfo("", "", "", ApplicationCommandInfo::CommandFlags::hiddenFromKeyEditor);
  }

  bool perform(InvocationInfo const &info) override {
    Component *current = fMainWindow->getContentComponent();
    switch (info.commandID) {
    case gui::toJ2BConfig: {
      auto provider = dynamic_cast<ChooseInputStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto state = provider->getChooseInputState();
      if (!state) {
        return false;
      }
      if (state->fType != InputType::Java) {
        return false;
      }
      auto config = new component::j2b::J2BConfig(*state);
      fMainWindow->setContentOwned(config, true);
      return true;
    }
    case gui::toJ2BChooseInput: {
      std::optional<ChooseInputState> state;
      auto provider = dynamic_cast<ChooseInputStateProvider *>(current);
      if (provider) {
        state = provider->getChooseInputState();
      }
      auto chooseInput = new component::ChooseJavaInput(state);
      fMainWindow->setContentOwned(chooseInput, true);
      fMainWindow->setName(getApplicationName() + " : " + TRANS("Java to Bedrock"));
      return true;
    }
    case gui::toJ2BConvert: {
      auto provider = dynamic_cast<J2BConfigStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto convert = new component::j2b::J2BConvertProgress(provider->getConfigState());
      fMainWindow->setContentOwned(convert, true);
      return true;
    }
    case gui::toJ2BChooseOutput: {
      auto provider = dynamic_cast<J2BConvertStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto chooseOutput = new component::ChooseBedrockOutput(provider->getConvertState());
      fMainWindow->setContentOwned(chooseOutput, true);
      return true;
    }
    case gui::toJ2BCopy: {
      auto provider = dynamic_cast<J2BChooseOutputStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto copy = new component::CopyBedrockArtifactProgress(provider->getChooseOutputState());
      fMainWindow->setContentOwned(copy, true);
      return true;
    }
    case gui::toModeSelect: {
      auto modeSelect = new component::ModeSelect;
      fMainWindow->setContentOwned(modeSelect, true);
      fMainWindow->setName(Application::getApplicationName());
      return true;
    }
    case gui::toB2JChooseInput: {
      std::optional<ChooseInputState> state;
      auto provider = dynamic_cast<ChooseInputStateProvider *>(current);
      if (provider) {
        state = provider->getChooseInputState();
      }
      auto chooseInput = new component::ChooseBedrockInput(state);
      fMainWindow->setContentOwned(chooseInput, true);
      fMainWindow->setName(getApplicationName() + " : " + TRANS("Bedrock to Java"));
      return true;
    }
    case gui::toB2JConfig: {
      auto provider = dynamic_cast<ChooseInputStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto state = provider->getChooseInputState();
      if (!state) {
        return false;
      }
      if (state->fType != InputType::Bedrock) {
        return false;
      }
      auto config = new component::b2j::B2JConfig(*state);
      fMainWindow->setContentOwned(config, true);
      return true;
    }
    case gui::toB2JConvert: {
      auto provider = dynamic_cast<B2JConfigStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto convert = new component::b2j::B2JConvertProgress(provider->getConfigState());
      fMainWindow->setContentOwned(convert, true);
      return true;
    }
    case gui::toB2JChooseOutput: {
      auto provider = dynamic_cast<B2JConvertStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto chooseOutput = new component::ChooseJavaOutput(provider->getConvertState());
      fMainWindow->setContentOwned(chooseOutput, true);
      return true;
    }
    case gui::toB2JCopy: {
      auto provider = dynamic_cast<B2JChooseOutputStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto copy = new component::CopyJavaArtifactProgress(provider->getChooseOutputState());
      fMainWindow->setContentOwned(copy, true);
      return true;
    }
    default:
      return JUCEApplication::perform(info);
    }
  }

private:
  std::unique_ptr<component::MainWindow> fMainWindow;
  SharedResourcePointer<TooltipWindow> fTooltipWindow;
  std::unique_ptr<je2be::gui::LookAndFeel> fLaf;
};

} // namespace je2be::gui

START_JUCE_APPLICATION(je2be::gui::Application)
