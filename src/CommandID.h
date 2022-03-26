#pragma once

namespace je2be::gui {

enum {
  toModeSelect = 1,

  toChooseJavaInput,
  toChooseBedrockInput,
  toChooseXbox360InputToJava,
  toChooseXbox360InputToBedrock,

  toJ2BConfig,
  toJ2BConvert,

  toB2JConfig,
  toB2JConvert,

  toXbox360ToBedrockConfig,
  toXbox360ToJavaConfig,

  toChooseBedrockOutput,
  toChooseJavaOutput,

  toCopyBedrockArtifact,
  toCopyJavaArtifact,
};

}
