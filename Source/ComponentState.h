#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <optional>
#include <unordered_map>

namespace je2be::gui {

class J2BChooseInputState {
public:
  std::optional<juce::File> fInputDirectory;
};

class J2BChooseInputStateProvider {
public:
  virtual ~J2BChooseInputStateProvider() {}
  virtual J2BChooseInputState getChooseInputState() const = 0;
};

class J2BConfigState {
public:
  explicit J2BConfigState(J2BChooseInputState const &inputState)
      : fInputState(inputState) {}
  J2BChooseInputState const fInputState;
  enum class DirectoryStructure {
    Vanilla,
    Paper,
  };
  DirectoryStructure fStructure = DirectoryStructure::Vanilla;
};

class J2BConfigStateProvider {
public:
  virtual ~J2BConfigStateProvider() {}
  virtual J2BConfigState getConfigState() const = 0;
};

class J2BConvertStatistics {
public:
  J2BConvertStatistics() = default;

  std::unordered_map<uint32_t, uint64_t> fChunkDataVersions;
  uint64_t fNumChunks = 0;
  uint64_t fNumBlockEntities = 0;
  uint64_t fNumEntities = 0;
};

class J2BConvertState {
public:
  explicit J2BConvertState(J2BConfigState const &configState) : fConfigState(configState) {}
  J2BConfigState const fConfigState;
  juce::File fOutputDirectory;
  J2BConvertStatistics fStat;
};

class J2BConvertStateProvider {
public:
  virtual ~J2BConvertStateProvider() {}
  virtual J2BConvertState getConvertState() const = 0;
};

enum class J2BOutputFormat {
  Directory,
  MCWorld,
};

class J2BChooseOutputState {
public:
  explicit J2BChooseOutputState(J2BConvertState const &convertState)
      : fConvertState(convertState), fFormat(J2BOutputFormat::Directory) {}

  J2BConvertState const fConvertState;
  J2BOutputFormat fFormat;
  std::optional<juce::File> fCopyDestination;
};

class J2BChooseOutputStateProvider {
public:
  virtual ~J2BChooseOutputStateProvider() {}
  virtual J2BChooseOutputState getChooseOutputState() const = 0;
};

class B2JChooseInputState {
public:
  std::optional<juce::File> fInputFileOrDirectory;
};

class B2JChooseInputStateProvider {
public:
  virtual ~B2JChooseInputStateProvider() {}
  virtual B2JChooseInputState getChooseInputState() const = 0;
};

class B2JConfigState {
public:
  explicit B2JConfigState(B2JChooseInputState const &inputState)
      : fInputState(inputState) {}
  B2JChooseInputState const fInputState;
};

class B2JConfigStateProvider {
public:
  virtual ~B2JConfigStateProvider() {}
  virtual B2JConfigState getConfigState() const = 0;
};

class B2JConvertState {
public:
  explicit B2JConvertState(B2JConfigState const &configState) : fConfigState(configState) {}
  B2JConfigState const fConfigState;
  juce::File fOutputDirectory;
};

class B2JConvertStateProvider {
public:
  virtual ~B2JConvertStateProvider() {}
  virtual B2JConvertState getConvertState() const = 0;
};

class B2JChooseOutputState {
public:
  explicit B2JChooseOutputState(B2JConvertState const &convertState)
      : fConvertState(convertState) {}

  B2JConvertState const fConvertState;
  std::optional<juce::File> fCopyDestination;
};

class B2JChooseOutputStateProvider {
public:
  virtual ~B2JChooseOutputStateProvider() {}
  virtual B2JChooseOutputState getChooseOutputState() const = 0;
};

} // namespace je2be::gui
