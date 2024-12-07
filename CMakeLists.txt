cmake_minimum_required(VERSION 3.16)
project(je2be-desktop VERSION 5.5.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BUILD_SHARED_LIBS OFF)
add_definitions(
  /DUNICODE
  /D_UNICODE
  /utf-8
  /we4828
  /wd4100 # Suppress "unreferenced formal parameter" warnings
)
add_compile_options(
  /Ot # Favor Fast Code
  /Oi # Generate Intrinsic Functions
  $<IF:$<CONFIG:Debug>,/ZI,/Zi>
  $<IF:$<CONFIG:Debug>,,/GL> # Enable: Whole program optimization
  $<IF:$<CONFIG:Debug>,/MDd,/MD>
  /MP
  /Zc:__cplusplus # Enable updated __cplusplus macro
  /Zc:preprocessor # Enable preprocessor conformance mode
)
add_link_options(
  /DEBUG
  $<IF:$<CONFIG:Debug>,,/LTCG> # Enable: Link-time code generation
)

include(cmake/CPM.cmake)

CPMAddPackage(
  NAME mimalloc
  GITHUB_REPOSITORY microsoft/mimalloc
  VERSION 2.1.7
  OPTIONS "MI_OVERRIDE ON"
          "MI_BUILD_TESTS OFF"
          "MI_BUILD_OBJECT OFF"
          "MI_BUILD_STATIC OFF"
          "MI_WIN_REDIRECT ON")

add_subdirectory(ext/je2be-core)

add_subdirectory(ext/JUCE)

juce_add_gui_app(je2be-desktop
  ICON_BIG "resource/icon-large.png"
  ICON_SMALL "resource/icon-small.png"
  PRODUCT_NAME "je2be")

set(je2be_desktop_files
  #begin je2be-desktop
  src/AccountScanThread.cpp
  src/AccountScanThread.h
  src/Application.cpp
  src/AsyncUpdaterWith.h
  src/BedrockGameDirectoryScanWorker.cpp
  src/BedrockGameDirectoryScanWorker.h
  src/CommandID.h
  src/ComponentState.h
  src/Constants.h
  src/File.h
  src/GameDirectory.h
  src/JavaGameDirectoryScanWorker.cpp
  src/JavaGameDirectoryScanWorker.h
  src/LocalizationHelper.cpp
  src/LocalizationHelper.h
  src/LookAndFeel.h
  src/PS3GameDirectoryScanWorker.cpp
  src/PS3GameDirectoryScanWorker.h
  src/Status.hpp
  src/TaskbarProgress.cpp
  src/TaskbarProgress.h
  src/TemporaryDirectory.cpp
  src/TemporaryDirectory.h
  src/Thread.h
  src/Xbox360GameDirectoryScanWorker.cpp
  src/Xbox360GameDirectoryScanWorker.h
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
  src/component/ChoosePS3Input.cpp
  src/component/ChoosePS3Input.h
  src/component/ChooseXbox360Input.cpp
  src/component/ChooseXbox360Input.h
  src/component/ConvertProgress.cpp
  src/component/ConvertProgress.h
  src/component/CopyBedrockArtifactProgress.cpp
  src/component/CopyBedrockArtifactProgress.h
  src/component/CopyJavaArtifactProgress.cpp
  src/component/CopyJavaArtifactProgress.h
  src/component/DrawableText.h
  src/component/MainWindow.cpp
  src/component/MainWindow.h
  src/component/ModeSelect.cpp
  src/component/ModeSelect.h
  src/component/SearchLabel.cpp
  src/component/SearchLabel.h
  src/component/TextButton.h
  src/component/ToBedrockConfig.cpp
  src/component/ToBedrockConfig.h
  src/component/ToJavaConfig.cpp
  src/component/ToJavaConfig.h
  src/component/b2j/B2JConvertProgress.cpp
  src/component/b2j/B2JConvertProgress.h
  src/component/j2b/J2BConvertProgress.cpp
  src/component/j2b/J2BConvertProgress.h
  src/component/p2b/P2BConvertProgress.cpp
  src/component/p2b/P2BConvertProgress.h
  src/component/p2j/P2JConvertProgress.cpp
  src/component/p2j/P2JConvertProgress.h
  src/component/x2b/X2BConvertProgress.cpp
  src/component/x2b/X2BConvertProgress.h
  src/component/x2j/X2JConvertProgress.cpp
  src/component/x2j/X2JConvertProgress.h
  #end je2be-desktop
)

target_sources(je2be-desktop
  PRIVATE
  ${je2be_desktop_files})

target_compile_definitions(je2be-desktop
  PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:je2be-desktop,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:je2be-desktop,JUCE_VERSION>"
    JUCE_DISPLAY_SPLASH_SCREEN=0
    NOMINMAX
    WIN32_LEAN_AND_MEAN)

juce_add_binary_data(je2be-desktop-data
  SOURCES
    resource/icon-large.png
    resource/ja-JP.lang
    resource/zh-Hans.lang)

target_link_libraries(je2be-desktop
  PRIVATE
    je2be
    mimalloc
    je2be-desktop-data
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

add_custom_command(TARGET je2be-desktop POST_BUILD
  COMMAND
    "$<$<CONFIG:Release>:${CMAKE_COMMAND}>"
    "$<$<CONFIG:Release>:-E>"
    "$<$<CONFIG:Release>:copy>"
    "$<$<CONFIG:Release>:${CMAKE_BINARY_DIR}/_deps/mimalloc-build/\$(Configuration)/mimalloc.dll>"
    "$<$<CONFIG:Release>:${CMAKE_BINARY_DIR}/Package/je2be-desktop/>"
  COMMAND
    "$<$<CONFIG:Release>:${CMAKE_COMMAND}>"
    "$<$<CONFIG:Release>:-E>"
    "$<$<CONFIG:Release>:copy>"
    "$<$<CONFIG:Release>:${CMAKE_BINARY_DIR}/_deps/mimalloc-build/\$(Configuration)/mimalloc-redirect.dll>"
    "$<$<CONFIG:Release>:${CMAKE_BINARY_DIR}/Package/je2be-desktop/>"
  COMMENT "Copy mimalloc dlls to package output directory")

add_custom_command(TARGET je2be-desktop POST_BUILD
  COMMAND
    "${CMAKE_COMMAND}"
    -E
    copy
    "${mimalloc_BINARY_DIR}/\$(Configuration)/mimalloc.dll"
    "$<TARGET_FILE_DIR:je2be-desktop>/"
  COMMAND
    "${CMAKE_COMMAND}"
    -E
    copy
    "${mimalloc_BINARY_DIR}/\$(Configuration)/mimalloc-redirect.dll"
    "$<TARGET_FILE_DIR:je2be-desktop>/"
  COMMENT "Copy mimalloc dlls to exe output directory")

source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${je2be_desktop_files})
