#pragma once

#include <JuceHeader.h>
#include <optional>

class ChooseInputState {
public:
  std::optional<File> fInputDirectory;
};

class ChooseInputStateProvider {
public:
  virtual ~ChooseInputStateProvider() {}
  virtual ChooseInputState getChooseInputState() const = 0;
};

class ConfigState {
public:
  explicit ConfigState(ChooseInputState const &inputState)
      : fInputState(inputState) {}
  ChooseInputState const fInputState;
};

class ConfigStateProvider {
public:
  virtual ~ConfigStateProvider() {}
  virtual ConfigState getConfigState() const = 0;
};

class ConvertState {
public:
  explicit ConvertState(ConfigState const &configState)
      : fConfigState(configState) {}
  ConfigState const fConfigState;
  File fOutputDirectory;
};

class ConvertStateProvider {
public:
  virtual ~ConvertStateProvider() {}
  virtual ConvertState getConvertState() const = 0;
};

class ChooseOutputState {
public:
  explicit ChooseOutputState(ConvertState const &convertState)
      : fConvertState(convertState) {}
  ConvertState const fConvertState;
  std::optional<File> fCopyDestinationDirectory;
};

class ChooseOutputStateProvider {
public:
  virtual ~ChooseOutputStateProvider() {}
  virtual ChooseOutputState getChooseOutputState() const = 0;
};
