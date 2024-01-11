#pragma once

namespace je2be::desktop::commands {

enum {
  toModeSelect = 1,

  toChooseJavaInput,
  toChooseBedrockInput,
  toChooseXbox360InputToJava,
  toChooseXbox360InputToBedrock,
  toChoosePS3InputToJava,
  toChoosePS3InputToBedrock,

  toJ2BConfig,
  toJ2BConvert,

  toB2JConfig,
  toB2JConvert,

  toXbox360ToBedrockConfig,
  toXbox360ToJavaConvert,

  toXbox360ToJavaConfig,
  toXbox360ToBedrockConvert,

  toPS3ToJavaConfig,
  toPS3ToBedrockConfig,

  toChooseBedrockOutput,
  toChooseJavaOutput,

  toCopyBedrockArtifact,
  toCopyJavaArtifact,
};

}
