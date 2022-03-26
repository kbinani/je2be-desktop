cmake_minimum_required(VERSION 3.16)
project(je2be-desktop VERSION 4.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BUILD_SHARED_LIBS OFF)
add_definitions(
  /DUNICODE
  /D_UNICODE
  /source-charset:utf-8
  /we4828
  /wd4100 # Suppress "unreferenced formal parameter" warnings
)
add_compile_options(
  $<$<CONFIG:Debug>:/MTd>
  $<$<CONFIG:Release>:/MT>
  $<$<CONFIG:RelWithDebInfo>:/MT>
  $<$<CONFIG:MinSizeRel>:/MT>)

add_subdirectory(ext/je2be)
add_subdirectory(ext/JUCE)

juce_add_gui_app(je2be-desktop
  ICON_BIG "resource/icon-large.png"
  ICON_SMALL "resource/icon-small.png"
  PRODUCT_NAME "je2be")

set(je2be_desktop_files
  #begin je2be-desktop
  src/Application.cpp
  src/CommandID.h
  src/ComponentState.h
  src/Constants.h
  src/File.h
  src/GameDirectory.h
  src/GameDirectoryScanThreadBedrock.cpp
  src/GameDirectoryScanThreadBedrock.h
  src/GameDirectoryScanThreadJava.cpp
  src/GameDirectoryScanThreadJava.h
  src/LocalizationHelper.cpp
  src/LocalizationHelper.h
  src/LookAndFeel.h
  src/TaskbarProgress.cpp
  src/TaskbarProgress.h
  src/TemporaryDirectory.cpp
  src/TemporaryDirectory.h
  src/component/About.cpp
  src/component/About.h
  src/component/ChooseBedrockInput.cpp
  src/component/ChooseBedrockInput.h
  src/component/ChooseBedrockOutput.cpp
  src/component/ChooseBedrockOutput.h
  src/component/ChooseJavaInput.cpp
  src/component/ChooseJavaInput.h
  src/component/ChooseJavaOutput.cpp
  src/component/ChooseJavaOutput.h
  src/component/CopyBedrockArtifactProgress.cpp
  src/component/CopyBedrockArtifactProgress.h
  src/component/CopyJavaArtifactProgress.cpp
  src/component/CopyJavaArtifactProgress.h
  src/component/DrawableText.h
  src/component/MainWindow.cpp
  src/component/MainWindow.h
  src/component/ModeSelect.cpp
  src/component/ModeSelect.h
  src/component/TextButton.h
  src/component/b2j/B2JConfig.cpp
  src/component/b2j/B2JConfig.h
  src/component/b2j/B2JConvertProgress.cpp
  src/component/b2j/B2JConvertProgress.h
  src/component/j2b/J2BConfig.cpp
  src/component/j2b/J2BConfig.h
  src/component/j2b/J2BConvertProgress.cpp
  src/component/j2b/J2BConvertProgress.h
  #end je2be-desktop
)

target_sources(je2be-desktop
  PRIVATE
  ${je2be_desktop_files}
)

target_compile_definitions(je2be-desktop
  PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:je2be-desktop,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:je2be-desktop,JUCE_VERSION>"
    JUCE_DISPLAY_SPLASH_SCREEN=0
    NOMINMAX
    WIN32_LEAN_AND_MEAN)

target_compile_options(je2be-desktop PRIVATE $<$<CONFIG:Release>:/Zi>)
target_link_options(je2be-desktop PRIVATE $<$<CONFIG:Release>:/DEBUG>)
target_compile_options(leveldb PRIVATE $<$<CONFIG:Release>:/Zi>)
target_link_options(leveldb PRIVATE $<$<CONFIG:Release>:/DEBUG>)
target_compile_options(zlib PRIVATE $<$<CONFIG:Release>:/Zi>)
target_link_options(zlib PRIVATE $<$<CONFIG:Release>:/DEBUG>)

juce_add_binary_data(app-data
  SOURCES
    resource/icon-large.png
    resource/japanese.lang)

target_link_libraries(je2be-desktop
  PRIVATE
    je2be
    app-data
    juce::juce_gui_extra
  PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

target_include_directories(je2be-desktop PRIVATE src)

include_external_msproject(Package Package/Package.wapproj
  TYPE C7167F0D-BC9F-4E6E-AFE1-012C56B48DB5
  GUID 713653CB-7989-4C22-8171-0F1951271B8D)

configure_file(Builds/Package/Package.appxmanifest.in Package/Package.appxmanifest)

source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${je2be_desktop_files})
