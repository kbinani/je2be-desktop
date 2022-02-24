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

target_sources(je2be-gui
  PRIVATE
    Source/AboutComponent.cpp
    Source/AboutComponent.h
    Source/J2BChooseInputComponent.cpp
    Source/J2BChooseInputComponent.h
    Source/J2BChooseOutputComponent.cpp
    Source/J2BChooseOutputComponent.h
    Source/CommandID.h
    Source/ComponentState.h
    Source/J2BConfigComponent.cpp
    Source/J2BConfigComponent.h
    Source/Constants.h
    Source/J2BConvertProgressComponent.cpp
    Source/J2BConvertProgressComponent.h
    Source/J2BCopyProgressComponent.cpp
    Source/J2BCopyProgressComponent.h
    Source/LocalizationHelper.cpp
    Source/LocalizationHelper.h
    Source/Application.cpp
    Source/TemporaryDirectory.h
    Source/TemporaryDirectory.cpp
    Source/MainWindow.h
    Source/MainWindow.cpp
    Source/TaskbarProgress.h
    Source/TaskbarProgress.cpp
    Source/LookAndFeel.h
    Source/ModeSelectComponent.h
    Source/ModeSelectComponent.cpp
    Source/B2JChooseInputComponent.cpp
    Source/B2JChooseInputComponent.h
    Source/GameDirectories.h
    Source/B2JConfigComponent.cpp
    Source/B2JConfigComponent.h
    Source/B2JConvertProgressComponent.cpp
    Source/B2JConvertProgressComponent.h
    Source/B2JChooseOutputComponent.cpp
    Source/B2JChooseOutputComponent.h
    Source/B2JCopyProgressComponent.cpp
    Source/B2JCopyProgressComponent.h
    Source/File.h
    Source/TextButtonComponent.h
    Source/DrawableTextComponent.h)

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
