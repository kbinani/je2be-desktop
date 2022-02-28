cmake_minimum_required(VERSION 3.16)
project(je2be-gui VERSION 3.2.0)

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
  src/AboutComponent.cpp
  src/AboutComponent.h
  src/Application.cpp
  src/B2JChooseInputComponent.cpp
  src/B2JChooseInputComponent.h
  src/B2JChooseOutputComponent.cpp
  src/B2JChooseOutputComponent.h
  src/B2JConfigComponent.cpp
  src/B2JConfigComponent.h
  src/B2JConvertProgressComponent.cpp
  src/B2JConvertProgressComponent.h
  src/B2JCopyProgressComponent.cpp
  src/B2JCopyProgressComponent.h
  src/CommandID.h
  src/ComponentState.h
  src/Constants.h
  src/DrawableTextComponent.h
  src/File.h
  src/GameDirectory.h
  src/GameDirectoryScanThreadBedrock.cpp
  src/GameDirectoryScanThreadBedrock.h
  src/GameDirectoryScanThreadJava.cpp
  src/GameDirectoryScanThreadJava.h
  src/J2BChooseInputComponent.cpp
  src/J2BChooseInputComponent.h
  src/J2BChooseOutputComponent.cpp
  src/J2BChooseOutputComponent.h
  src/J2BConfigComponent.cpp
  src/J2BConfigComponent.h
  src/J2BConvertProgressComponent.cpp
  src/J2BConvertProgressComponent.h
  src/J2BCopyProgressComponent.cpp
  src/J2BCopyProgressComponent.h
  src/LocalizationHelper.cpp
  src/LocalizationHelper.h
  src/LookAndFeel.h
  src/MainWindow.cpp
  src/MainWindow.h
  src/ModeSelectComponent.cpp
  src/ModeSelectComponent.h
  src/TaskbarProgress.cpp
  src/TaskbarProgress.h
  src/TemporaryDirectory.cpp
  src/TemporaryDirectory.h
  src/TextButtonComponent.h
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
    Resource/icon-large.png
    Resource/japanese.lang)

target_link_libraries(je2be-gui
  PRIVATE
    je2be
    app-data
    juce::juce_gui_extra
  PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

include_external_msproject(Package Package/Package.wapproj
  TYPE C7167F0D-BC9F-4E6E-AFE1-012C56B48DB5
  GUID 713653CB-7989-4C22-8171-0F1951271B8D)

configure_file(Builds/Package/Package.appxmanifest.in Package/Package.appxmanifest)

setup_source_groups("${je2be_gui_files}" "${CMAKE_CURRENT_SOURCE_DIR}")
