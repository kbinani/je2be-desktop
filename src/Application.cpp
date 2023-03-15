#include "CommandID.h"
#include "Constants.h"
#include "LocalizationHelper.h"
#include "LookAndFeel.h"
#include "TemporaryDirectory.h"
#include "component/ChooseBedrockInput.h"
#include "component/ChooseBedrockOutput.h"
#include "component/ChooseJavaInput.h"
#include "component/ChooseJavaOutput.h"
#include "component/ChooseXbox360Input.h"
#include "component/CopyBedrockArtifactProgress.h"
#include "component/CopyJavaArtifactProgress.h"
#include "component/MainWindow.h"
#include "component/b2j/B2JConfig.h"
#include "component/b2j/B2JConvertProgress.h"
#include "component/j2b/J2BConfig.h"
#include "component/j2b/J2BConvertProgress.h"
#include "component/x2b/X2BConfig.h"
#include "component/x2b/X2BConvertProgress.h"
#include "component/x2j/X2JConfig.h"
#include "component/x2j/X2JConvertProgress.h"

#include <mimalloc.h>

using namespace juce;

namespace je2be::desktop {

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
    (void)mi_version();
    fLaf.reset(new je2be::desktop::LookAndFeel);
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
    commands.addArray({commands::toJ2BConfig,
                       commands::toChooseJavaInput,
                       commands::toJ2BConvert,
                       commands::toChooseBedrockOutput,
                       commands::toCopyBedrockArtifact,
                       commands::toModeSelect,
                       commands::toChooseBedrockInput,
                       commands::toB2JConfig,
                       commands::toB2JConvert,
                       commands::toChooseJavaOutput,
                       commands::toCopyJavaArtifact,
                       commands::toChooseXbox360InputToBedrock,
                       commands::toChooseXbox360InputToJava,
                       commands::toXbox360ToJavaConfig,
                       commands::toXbox360ToJavaConvert,
                       commands::toXbox360ToBedrockConfig,
                       commands::toXbox360ToBedrockConvert});
  }

  void getCommandInfo(CommandID commandID, ApplicationCommandInfo &result) override {
    result.setInfo("", "", "", ApplicationCommandInfo::CommandFlags::hiddenFromKeyEditor);
  }

  bool perform(InvocationInfo const &info) override {
    Component *current = fMainWindow->getContentComponent();
    switch (info.commandID) {
    case commands::toJ2BConfig: {
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
    case commands::toChooseJavaInput: {
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
    case commands::toJ2BConvert: {
      auto provider = dynamic_cast<J2BConfigStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto convert = new component::j2b::J2BConvertProgress(provider->getConfigState());
      fMainWindow->setContentOwned(convert, true);
      return true;
    }
    case commands::toChooseBedrockOutput: {
      auto provider = dynamic_cast<BedrockConvertedStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto state = provider->getConvertedState();
      if (!state) {
        return false;
      }
      auto chooseOutput = new component::ChooseBedrockOutput(*state);
      fMainWindow->setContentOwned(chooseOutput, true);
      return true;
    }
    case commands::toCopyBedrockArtifact: {
      auto provider = dynamic_cast<BedrockOutputChoosenStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto copy = new component::CopyBedrockArtifactProgress(provider->getBedrockOutputChoosenState());
      fMainWindow->setContentOwned(copy, true);
      return true;
    }
    case commands::toModeSelect: {
      auto modeSelect = new component::ModeSelect;
      fMainWindow->setContentOwned(modeSelect, true);
      fMainWindow->setName(Application::getApplicationName());
      return true;
    }
    case commands::toChooseBedrockInput: {
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
    case commands::toB2JConfig: {
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
    case commands::toB2JConvert: {
      auto provider = dynamic_cast<B2JConfigStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto convert = new component::b2j::B2JConvertProgress(provider->getConfigState());
      fMainWindow->setContentOwned(convert, true);
      return true;
    }
    case commands::toChooseJavaOutput: {
      auto provider = dynamic_cast<JavaConvertedStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto state = provider->getConvertedState();
      if (!state) {
        return false;
      }
      auto chooseOutput = new component::ChooseJavaOutput(*state);
      fMainWindow->setContentOwned(chooseOutput, true);
      return true;
    }
    case commands::toCopyJavaArtifact: {
      auto provider = dynamic_cast<JavaOutputChoosenStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto copy = new component::CopyJavaArtifactProgress(provider->getJavaOutputChoosenState());
      fMainWindow->setContentOwned(copy, true);
      return true;
    }
    case commands::toChooseXbox360InputToBedrock:
    case commands::toChooseXbox360InputToJava: {
      std::optional<ChooseInputState> state;
      auto provider = dynamic_cast<ChooseInputStateProvider *>(current);
      if (provider) {
        state = provider->getChooseInputState();
      }
      CommandID destination;
      String title;
      if (info.commandID == commands::toChooseXbox360InputToBedrock) {
        destination = commands::toXbox360ToBedrockConfig;
        title = TRANS("Xbox360 to Bedrock");
      } else {
        destination = commands::toXbox360ToJavaConfig;
        title = TRANS("Xbox360 to Java");
      }
      auto chooseInput = new component::ChooseXbox360Input(destination, state);
      fMainWindow->setContentOwned(chooseInput, true);
      fMainWindow->setName(getApplicationName() + " : " + title);
      return true;
    }
    case commands::toXbox360ToJavaConfig: {
      auto provider = dynamic_cast<ChooseInputStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto state = provider->getChooseInputState();
      if (!state) {
        return false;
      }
      if (state->fType != InputType::Xbox360) {
        return false;
      }
      auto config = new component::x2j::X2JConfig(*state);
      fMainWindow->setContentOwned(config, true);
      return true;
    }
    case commands::toXbox360ToJavaConvert: {
      auto provider = dynamic_cast<X2JConfigStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto convert = new component::x2j::X2JConvertProgress(provider->getConfigState());
      fMainWindow->setContentOwned(convert, true);
      return true;
    }
    case commands::toXbox360ToBedrockConfig: {
      auto provider = dynamic_cast<ChooseInputStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto state = provider->getChooseInputState();
      if (!state) {
        return false;
      }
      if (state->fType != InputType::Xbox360) {
        return false;
      }
      auto config = new component::x2b::X2BConfig(*state);
      fMainWindow->setContentOwned(config, true);
      return true;
    }
    case commands::toXbox360ToBedrockConvert: {
      auto provider = dynamic_cast<X2BConfigStateProvider *>(current);
      if (!provider) {
        return false;
      }
      auto convert = new component::x2b::X2BConvertProgress(provider->getConfigState());
      fMainWindow->setContentOwned(convert, true);
      return true;
    }
    default:
      return JUCEApplication::perform(info);
    }
  }

private:
  std::unique_ptr<component::MainWindow> fMainWindow;
  SharedResourcePointer<TooltipWindow> fTooltipWindow;
  std::unique_ptr<je2be::desktop::LookAndFeel> fLaf;
};

} // namespace je2be::desktop

START_JUCE_APPLICATION(je2be::desktop::Application)
