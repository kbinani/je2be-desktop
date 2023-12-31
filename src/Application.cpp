#include "CommandID.h"
#include "Constants.h"
#include "LocalizationHelper.h"
#include "LookAndFeel.h"
#include "TemporaryDirectory.h"
#include "Thread.h"
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
#include <objbase.h>

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
    (void)CoInitialize(nullptr);

    fLaf.reset(new je2be::desktop::LookAndFeel);
    LookAndFeel::setDefaultLookAndFeel(fLaf.get());

    juce::LocalisedStrings::setCurrentMappings(LocalizationHelper::CurrentLocalisedStrings());

    TemporaryDirectory::CleanupAsync();

    fMainWindow.reset(new component::MainWindow(getApplicationName()));
    fMainWindow->setVisible(true);
  }

  void shutdown() override {
    fMainWindow = nullptr;
    Thread::Wait();
    CoUninitialize();
  }

  void systemRequestedQuit() override {
    quit();
  }

  void getAllCommands(juce::Array<juce::CommandID> &commands) override {
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

  void getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo &result) override {
    result.setInfo("", "", "", juce::ApplicationCommandInfo::CommandFlags::hiddenFromKeyEditor);
  }

  bool perform(InvocationInfo const &info) override {
    auto current = fMainWindowContent;
    switch (info.commandID) {
    case commands::toJ2BConfig: {
      auto provider = dynamic_cast<ChooseInputStateProvider *>(current.get());
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
      auto config = std::make_shared<component::j2b::J2BConfig>(*state);
      fMainWindow->setContentNonOwned(config.get(), true);
      fMainWindowContent = config;
      return true;
    }
    case commands::toChooseJavaInput: {
      std::optional<ChooseInputState> state;
      auto provider = dynamic_cast<ChooseInputStateProvider *>(current.get());
      if (provider) {
        state = provider->getChooseInputState();
      }
      auto chooseInput = std::make_shared<component::ChooseJavaInput>(state);
      fMainWindow->setContentNonOwned(chooseInput.get(), true);
      fMainWindowContent = chooseInput;
      fMainWindow->setName(getApplicationName() + " : " + TRANS("Java to Bedrock"));
      return true;
    }
    case commands::toJ2BConvert: {
      auto provider = dynamic_cast<J2BConfigStateProvider *>(current.get());
      if (!provider) {
        return false;
      }
      auto convert = std::make_shared<component::j2b::J2BConvertProgress>(provider->getConfigState());
      fMainWindow->setContentNonOwned(convert.get(), true);
      fMainWindowContent = convert;
      return true;
    }
    case commands::toChooseBedrockOutput: {
      auto provider = dynamic_cast<BedrockConvertedStateProvider *>(current.get());
      if (!provider) {
        return false;
      }
      auto state = provider->getConvertedState();
      if (!state) {
        return false;
      }
      auto chooseOutput = std::make_shared<component::ChooseBedrockOutput>(*state);
      fMainWindow->setContentNonOwned(chooseOutput.get(), true);
      fMainWindowContent = chooseOutput;
      return true;
    }
    case commands::toCopyBedrockArtifact: {
      auto provider = dynamic_cast<BedrockOutputChoosenStateProvider *>(current.get());
      if (!provider) {
        return false;
      }
      auto copy = std::make_shared<component::CopyBedrockArtifactProgress>(provider->getBedrockOutputChoosenState());
      fMainWindow->setContentNonOwned(copy.get(), true);
      fMainWindowContent = copy;
      return true;
    }
    case commands::toModeSelect: {
      auto modeSelect = std::make_shared<component::ModeSelect>();
      fMainWindow->setContentNonOwned(modeSelect.get(), true);
      fMainWindowContent = modeSelect;
      fMainWindow->setName(Application::getApplicationName());
      return true;
    }
    case commands::toChooseBedrockInput: {
      std::optional<ChooseInputState> state;
      auto provider = dynamic_cast<ChooseInputStateProvider *>(current.get());
      if (provider) {
        state = provider->getChooseInputState();
      }
      auto chooseInput = std::make_shared<component::ChooseBedrockInput>(state);
      fMainWindow->setContentNonOwned(chooseInput.get(), true);
      fMainWindowContent = chooseInput;
      fMainWindow->setName(getApplicationName() + " : " + TRANS("Bedrock to Java"));
      return true;
    }
    case commands::toB2JConfig: {
      auto provider = dynamic_cast<ChooseInputStateProvider *>(current.get());
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
      auto config = std::make_shared<component::b2j::B2JConfig>(*state);
      fMainWindow->setContentNonOwned(config.get(), true);
      fMainWindowContent = config;
      return true;
    }
    case commands::toB2JConvert: {
      auto provider = dynamic_cast<B2JConfigStateProvider *>(current.get());
      if (!provider) {
        return false;
      }
      auto convert = std::make_shared<component::b2j::B2JConvertProgress>(provider->getConfigState());
      fMainWindow->setContentNonOwned(convert.get(), true);
      fMainWindowContent = convert;
      return true;
    }
    case commands::toChooseJavaOutput: {
      auto provider = dynamic_cast<JavaConvertedStateProvider *>(current.get());
      if (!provider) {
        return false;
      }
      auto state = provider->getConvertedState();
      if (!state) {
        return false;
      }
      auto chooseOutput = std::make_shared<component::ChooseJavaOutput>(*state);
      fMainWindow->setContentNonOwned(chooseOutput.get(), true);
      fMainWindowContent = chooseOutput;
      return true;
    }
    case commands::toCopyJavaArtifact: {
      auto provider = dynamic_cast<JavaOutputChoosenStateProvider *>(current.get());
      if (!provider) {
        return false;
      }
      auto copy = std::make_shared<component::CopyJavaArtifactProgress>(provider->getJavaOutputChoosenState());
      fMainWindow->setContentNonOwned(copy.get(), true);
      fMainWindowContent = copy;
      return true;
    }
    case commands::toChooseXbox360InputToBedrock:
    case commands::toChooseXbox360InputToJava: {
      std::optional<ChooseInputState> state;
      auto provider = dynamic_cast<ChooseInputStateProvider *>(current.get());
      if (provider) {
        state = provider->getChooseInputState();
      }
      juce::CommandID destination;
      juce::String title;
      if (info.commandID == commands::toChooseXbox360InputToBedrock) {
        destination = commands::toXbox360ToBedrockConfig;
        title = TRANS("Xbox360 to Bedrock");
      } else {
        destination = commands::toXbox360ToJavaConfig;
        title = TRANS("Xbox360 to Java");
      }
      auto chooseInput = std::make_shared<component::ChooseXbox360Input>(destination, state);
      fMainWindow->setContentNonOwned(chooseInput.get(), true);
      fMainWindowContent = chooseInput;
      fMainWindow->setName(getApplicationName() + " : " + title);
      return true;
    }
    case commands::toXbox360ToJavaConfig: {
      auto provider = dynamic_cast<ChooseInputStateProvider *>(current.get());
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
      auto config = std::make_shared<component::x2j::X2JConfig>(*state);
      fMainWindow->setContentNonOwned(config.get(), true);
      fMainWindowContent = config;
      return true;
    }
    case commands::toXbox360ToJavaConvert: {
      auto provider = dynamic_cast<X2JConfigStateProvider *>(current.get());
      if (!provider) {
        return false;
      }
      auto convert = std::make_shared<component::x2j::X2JConvertProgress>(provider->getConfigState());
      fMainWindow->setContentNonOwned(convert.get(), true);
      fMainWindowContent = convert;
      return true;
    }
    case commands::toXbox360ToBedrockConfig: {
      auto provider = dynamic_cast<ChooseInputStateProvider *>(current.get());
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
      auto config = std::make_shared<component::x2b::X2BConfig>(*state);
      fMainWindow->setContentNonOwned(config.get(), true);
      fMainWindowContent = config;
      return true;
    }
    case commands::toXbox360ToBedrockConvert: {
      auto provider = dynamic_cast<X2BConfigStateProvider *>(current.get());
      if (!provider) {
        return false;
      }
      auto convert = std::make_shared<component::x2b::X2BConvertProgress>(provider->getConfigState());
      fMainWindow->setContentNonOwned(convert.get(), true);
      fMainWindowContent = convert;
      return true;
    }
    default:
      return JUCEApplication::perform(info);
    }
  }

private:
  std::unique_ptr<component::MainWindow> fMainWindow;
  std::shared_ptr<juce::Component> fMainWindowContent;
  juce::SharedResourcePointer<juce::TooltipWindow> fTooltipWindow;
  std::unique_ptr<je2be::desktop::LookAndFeel> fLaf;
};

} // namespace je2be::desktop

START_JUCE_APPLICATION(je2be::desktop::Application)
