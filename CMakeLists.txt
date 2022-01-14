cmake_minimum_required(VERSION 3.16)
project(je2be-gui VERSION 2.10.2)

if (NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()

set(BUILD_SHARED_LIBS OFF)
add_definitions(/DUNICODE /D_UNICODE)
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
    Source/Application.cpp
    Source/TemporaryDirectory.h
    Source/MainWindow.h
    Source/MainWindow.cpp
    Source/TaskbarProgress.h
    Source/TaskbarProgress.cpp
    Source/LookAndFeel.h)

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
