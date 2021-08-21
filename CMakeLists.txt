cmake_minimum_required(VERSION 3.16)
project(je2be-gui VERSION 2.3.0)

add_subdirectory(ext/je2be)
add_subdirectory(ext/JUCE)

add_definitions(/std:c++latest)

juce_add_gui_app(je2be-gui
  ICON_BIG "Resource/icon-large.png"
  ICON_SMALL "Resource/icon-small.png"
  PRODUCT_NAME "je2be")

target_sources(je2be-gui
  PRIVATE
    Source/AboutComponent.cpp
    Source/AboutComponent.h
    Source/ChooseInputComponent.cpp
    Source/ChooseInputComponent.h
    Source/ChooseOutputComponent.cpp
    Source/ChooseOutputComponent.h
    Source/CommandID.h
    Source/ComponentState.h
    Source/ConfigComponent.cpp
    Source/ConfigComponent.h
    Source/Constants.h
    Source/ConvertProgressComponent.cpp
    Source/ConvertProgressComponent.h
    Source/CopyProgressComponent.cpp
    Source/CopyProgressComponent.h
    Source/LocalizationHelper.cpp
    Source/LocalizationHelper.h
    Source/Main.cpp
    Source/TemporaryDirectory.h)

target_compile_definitions(je2be-gui
  PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:je2be-gui,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:je2be-gui,JUCE_VERSION>"
    JUCE_DISPLAY_SPLASH_SCREEN=0)

juce_add_binary_data(GuiAppData
  SOURCES
    Resource/icon-large.png
    Resource/icon-small.png
    Resource/japanese.lang)

target_link_libraries(je2be-gui
  PRIVATE
    GuiAppData
    juce::juce_gui_extra
    xxhash
    zlibstatic
    leveldb
  PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

target_include_directories(je2be-gui
  PRIVATE
    ext/je2be/include
    ext/je2be/ext/libminecraft-file/include
    ext/je2be/ext/zlib-ng
    ${CMAKE_BINARY_DIR}/ext/je2be/ext/zlib-ng
    ext/je2be/ext/leveldb
    ext/je2be/ext/leveldb/include
    ext/je2be/ext/xxHash
    ext/je2be/ext/thread-pool/include
    ext/je2be/ext/json/include)

include_external_msproject(Package Package/Package.wapproj
  TYPE C7167F0D-BC9F-4E6E-AFE1-012C56B48DB5
  GUID 713653CB-7989-4C22-8171-0F1951271B8D)
