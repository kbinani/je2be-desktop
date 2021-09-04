#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <optional>
#include <unordered_map>

namespace je2be::gui {

class ChooseInputState {
public:
  std::optional<juce::File> fInputDirectory;
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
  enum class DirectoryStructure {
    Vanilla,
    Paper,
  };
  DirectoryStructure fStructure = DirectoryStructure::Vanilla;
};

class ConfigStateProvider {
public:
  virtual ~ConfigStateProvider() {}
  virtual ConfigState getConfigState() const = 0;
};

class ConvertStatistics {
public:
  ConvertStatistics() = default;

  std::unordered_map<uint32_t, uint64_t> fChunkDataVersions;
  uint64_t fNumChunks = 0;
  uint64_t fNumBlockEntities = 0;
  uint64_t fNumEntities = 0;
};

class ConvertState {
public:
  explicit ConvertState(ConfigState const &configState) : fConfigState(configState) {}
  ConfigState const fConfigState;
  juce::File fOutputDirectory;
  ConvertStatistics fStat;
};

class ConvertStateProvider {
public:
  virtual ~ConvertStateProvider() {}
  virtual ConvertState getConvertState() const = 0;
};

enum class OutputFormat {
  Directory,
  MCWorld,
};

class ChooseOutputState {
public:
  explicit ChooseOutputState(ConvertState const &convertState)
      : fConvertState(convertState), fFormat(OutputFormat::Directory) {}

  ConvertState const fConvertState;
  OutputFormat fFormat;
  std::optional<juce::File> fCopyDestination;
};

class ChooseOutputStateProvider {
public:
  virtual ~ChooseOutputStateProvider() {}
  virtual ChooseOutputState getChooseOutputState() const = 0;
};

} // namespace je2be::gui
