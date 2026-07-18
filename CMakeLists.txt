cmake_minimum_required(VERSION 3.22)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(APPLE)
    execute_process(COMMAND uname -m OUTPUT_VARIABLE HOST_ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(NOT CMAKE_OSX_ARCHITECTURES)
        set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architectures for macOS")
    endif()
    if(NOT CMAKE_GENERATOR)
        find_program(NINJA_EXE ninja)
        if(NINJA_EXE)
            set(CMAKE_GENERATOR "Ninja" CACHE INTERNAL "")
        endif()
    endif()
endif()

project(Matrix-Simulator VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(JUCE_DIR_MACOS "/Volumes/Guillaume/Dev/SDKs/JUCE/")
set(JUCE_DIR_WINDOWS "C:/Users/Guillaume/Dev/SDKs/JUCE")
set(JUCE_DIR_LINUX "/home/guillaume/Dev/SDKs/JUCE")

set(JUCE_DIR_CANDIDATE "")
if(DEFINED JUCE_DIR AND NOT JUCE_DIR STREQUAL "" AND EXISTS "${JUCE_DIR}")
    set(JUCE_DIR_CANDIDATE "${JUCE_DIR}")
elseif(DEFINED ENV{JUCE_DIR} AND NOT "$ENV{JUCE_DIR}" STREQUAL "")
    set(JUCE_DIR_CANDIDATE "$ENV{JUCE_DIR}")
elseif(APPLE AND EXISTS "${JUCE_DIR_MACOS}")
    set(JUCE_DIR_CANDIDATE "${JUCE_DIR_MACOS}")
elseif(WIN32 AND EXISTS "${JUCE_DIR_WINDOWS}")
    set(JUCE_DIR_CANDIDATE "${JUCE_DIR_WINDOWS}")
elseif(UNIX AND NOT APPLE AND EXISTS "${JUCE_DIR_LINUX}")
    set(JUCE_DIR_CANDIDATE "${JUCE_DIR_LINUX}")
elseif(APPLE AND EXISTS "/Applications/JUCE")
    set(JUCE_DIR_CANDIDATE "/Applications/JUCE")
endif()

if(JUCE_DIR_CANDIDATE STREQUAL "")
    message(FATAL_ERROR "JUCE not found. Set JUCE_DIR or install JUCE.")
endif()

set(JUCE_DIR "${JUCE_DIR_CANDIDATE}" CACHE PATH "Path to JUCE installation" FORCE)
add_subdirectory(${JUCE_DIR} "${CMAKE_BINARY_DIR}/JUCE")

juce_add_gui_app(Matrix-Simulator
                 PRODUCT_NAME "Matrix-Simulator"
                 COMPANY_NAME "Ten Square Software"
                 BUNDLE_ID "com.tensquaresoftware.MatrixSimulator"
                 VERSION "${PROJECT_VERSION}"
                 ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Icons/Matrix-Simulator-Logo.png")

target_sources(Matrix-Simulator PRIVATE
    Source/Main.cpp
)

target_include_directories(Matrix-Simulator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_compile_definitions(Matrix-Simulator PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_STANDALONE_APPLICATION=1
)

target_link_libraries(Matrix-Simulator PRIVATE
    juce::juce_audio_devices
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)
