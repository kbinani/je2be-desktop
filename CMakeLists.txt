cmake_minimum_required(VERSION 3.16)
project(je2be-gui VERSION 4.0.0)

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

juce_add_gui_app(je2be-gui
  ICON_BIG "Resource/icon-large.png"
  ICON_SMALL "Resource/icon-small.png"
  PRODUCT_NAME "je2be")

set(je2be_gui_files
  #begin je2be-gui
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
  src/component/DrawableText.h
  src/component/MainWindow.cpp
  src/component/MainWindow.h
  src/component/ModeSelect.cpp
  src/component/ModeSelect.h
  src/component/TextButton.h
  src/component/b2j/B2JChooseOutput.cpp
  src/component/b2j/B2JChooseOutput.h
  src/component/b2j/B2JConfig.cpp
  src/component/b2j/B2JConfig.h
  src/component/b2j/B2JConvertProgress.cpp
  src/component/b2j/B2JConvertProgress.h
  src/component/b2j/B2JCopyProgress.cpp
  src/component/b2j/B2JCopyProgress.h
  src/component/j2b/J2BChooseInput.cpp
  src/component/j2b/J2BChooseInput.h
  src/component/j2b/J2BChooseOutput.cpp
  src/component/j2b/J2BChooseOutput.h
  src/component/j2b/J2BConfig.cpp
  src/component/j2b/J2BConfig.h
  src/component/j2b/J2BConvertProgress.cpp
  src/component/j2b/J2BConvertProgress.h
  src/component/j2b/J2BCopyProgress.cpp
  src/component/j2b/J2BCopyProgress.h
  #end je2be-gui
)

target_sources(je2be-gui
  PRIVATE
  ${je2be_gui_files}
)

target_compile_definitions(je2be-gui
  PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:je2be-gui,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:je2be-gui,JUCE_VERSION>"
    JUCE_DISPLAY_SPLASH_SCREEN=0
    NOMINMAX
    WIN32_LEAN_AND_MEAN)

target_compile_options(je2be-gui PRIVATE $<$<CONFIG:Release>:/Zi>)
target_link_options(je2be-gui PRIVATE $<$<CONFIG:Release>:/DEBUG>)
target_compile_options(leveldb PRIVATE $<$<CONFIG:Release>:/Zi>)
target_link_options(leveldb PRIVATE $<$<CONFIG:Release>:/DEBUG>)
target_compile_options(zlib PRIVATE $<$<CONFIG:Release>:/Zi>)
target_link_options(zlib PRIVATE $<$<CONFIG:Release>:/DEBUG>)

juce_add_binary_data(app-data
  SOURCES
    resource/icon-large.png
    resource/japanese.lang)

target_link_libraries(je2be-gui
  PRIVATE
    je2be
    app-data
    juce::juce_gui_extra
  PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

target_include_directories(je2be-gui PRIVATE src)

include_external_msproject(Package Package/Package.wapproj
  TYPE C7167F0D-BC9F-4E6E-AFE1-012C56B48DB5
  GUID 713653CB-7989-4C22-8171-0F1951271B8D)

configure_file(Builds/Package/Package.appxmanifest.in Package/Package.appxmanifest)

setup_source_groups("${je2be_gui_files}" "${CMAKE_CURRENT_SOURCE_DIR}")
