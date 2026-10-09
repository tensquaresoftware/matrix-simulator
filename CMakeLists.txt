# JUCE GUI app project (CMake). Do not remove cmake_minimum_required / project() order.
cmake_minimum_required(VERSION 3.22)

# IDE: compile_commands.json (Ninja / Makefile generators). Set before project() so generation picks it up.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Platform-specific configuration
if(APPLE)
    # Real host arch (uname -m): reliable on Apple Silicon even when CMake runs under Rosetta
    execute_process(COMMAND uname -m OUTPUT_VARIABLE HOST_ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
    # Architecture is set per-preset in CMakeUserPresets.json (arm64 for Apple Silicon, x86_64 for Intel)
    if(NOT CMAKE_OSX_ARCHITECTURES)
        set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architectures for macOS (arm64 or x86_64)")
    endif()
    # Preset vs host consistency: avoid wrong preset for the current machine
    # - Apple Silicon: reject "macOS Intel" (native); use "macOS Intel-Rosetta"
    # - Mac Intel: reject "macOS Intel-Rosetta" and "macOS Apple Silicon" (ARM); use "macOS Intel" (native)
    if(HOST_ARCH STREQUAL "arm64")
        # Rosetta builds use .../Intel-Rosetta, so they do not match .../Intel/?$ at end of path.
        if(CMAKE_BINARY_DIR MATCHES "Builds/macOS/Intel/?$")
            message(FATAL_ERROR
                "On Apple Silicon, do not use the 'macOS Intel' preset (macos-debug-x86_64; meant for native Mac Intel). Use an Intel-Rosetta binaryDir or stay on Apple Silicon presets. To list all presets: cmake --list-presets.")
        endif()
    elseif(HOST_ARCH STREQUAL "x86_64" OR HOST_ARCH STREQUAL "i386")
        if(CMAKE_BINARY_DIR MATCHES "Intel-Rosetta")
            message(FATAL_ERROR
                "On Mac Intel, do not use an Intel-Rosetta binaryDir (meant for Apple Silicon). Use 'macOS Intel' instead (macos-debug-x86_64). To list all presets: cmake --list-presets.")
        endif()
        if(CMAKE_BINARY_DIR MATCHES "Builds/macOS/ARM/?$")
            message(FATAL_ERROR
                "On Mac Intel, do not use the 'macOS Apple Silicon' preset (macos-debug-arm64). Use 'macOS Intel' instead (macos-debug-x86_64). To list all presets: cmake --list-presets.")
        endif()
    endif()
    # Use Ninja generator on macOS if not specified
    if(NOT CMAKE_GENERATOR)
        find_program(NINJA_EXE ninja)
        if(NINJA_EXE)
            set(CMAKE_GENERATOR "Ninja" CACHE INTERNAL "")
        endif()
    endif()
elseif(WIN32)
    # Use Visual Studio generator on Windows if not specified (prefer VS 2026; CMake 4.2+)
    if(NOT CMAKE_GENERATOR)
        set(CMAKE_GENERATOR "Visual Studio 18 2026" CACHE INTERNAL "")
    endif()
endif()

project(Matrix-Simulator VERSION 1.2.0)

# =============================================================================
# ARTEFACTS COPY CONFIGURATION
# COPY_TO_ARTEFACTS_DIR: ON/OFF — copies the app build output to a central custom folder
#   (ARTEFACTS_DIR_WINDOWS/MACOS/LINUX, organized by platform and architecture)
# Override at configure time: cmake .. -DUSER_COPY_TO_ARTEFACTS_DIR=ON
# =============================================================================

set(USER_COPY_TO_ARTEFACTS_DIR ON)

set(COPY_TO_ARTEFACTS_DIR ${USER_COPY_TO_ARTEFACTS_DIR} CACHE BOOL "Copy build outputs to central artefacts folder (organized by platform/architecture)" FORCE)

# Defaults; presets / -D may override via cache before this file runs.
if(NOT DEFINED ARTEFACTS_DIR_WINDOWS OR ARTEFACTS_DIR_WINDOWS STREQUAL "")
    set(ARTEFACTS_DIR_WINDOWS "C:/Users/Guillaume/Dropbox/Dev/Artefacts")
endif()
if(NOT DEFINED ARTEFACTS_DIR_MACOS OR ARTEFACTS_DIR_MACOS STREQUAL "")
    set(ARTEFACTS_DIR_MACOS "/Users/Guillaume/Library/CloudStorage/Dropbox/Dev/Artefacts/")
endif()
if(NOT DEFINED ARTEFACTS_DIR_LINUX OR ARTEFACTS_DIR_LINUX STREQUAL "")
    set(ARTEFACTS_DIR_LINUX "/home/guillaume/Dropbox/Dev/Artefacts")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(JUCE_DIR_WINDOWS "C:/Users/Guillaume/Dev/SDKs/JUCE")
set(JUCE_DIR_MACOS   "/Volumes/Guillaume/Dev/SDKs/JUCE-9")
set(JUCE_DIR_LINUX   "/home/guillaume/Dev/SDKs/JUCE")

# Reference JUCE: priority cache (preset/-D), then ENV, then workspace per-OS, then platform default
set(JUCE_DIR_CANDIDATE "")
if(DEFINED JUCE_DIR AND NOT JUCE_DIR STREQUAL "" AND EXISTS "${JUCE_DIR}")
    set(JUCE_DIR_CANDIDATE "${JUCE_DIR}")
elseif(DEFINED ENV{JUCE_DIR} AND NOT "$ENV{JUCE_DIR}" STREQUAL "")
    set(JUCE_DIR_CANDIDATE "$ENV{JUCE_DIR}")
elseif(WIN32 AND NOT JUCE_DIR_WINDOWS STREQUAL "" AND EXISTS "${JUCE_DIR_WINDOWS}")
    set(JUCE_DIR_CANDIDATE "${JUCE_DIR_WINDOWS}")
elseif(APPLE AND NOT JUCE_DIR_MACOS STREQUAL "" AND EXISTS "${JUCE_DIR_MACOS}")
    set(JUCE_DIR_CANDIDATE "${JUCE_DIR_MACOS}")
elseif(UNIX AND NOT APPLE AND NOT JUCE_DIR_LINUX STREQUAL "" AND EXISTS "${JUCE_DIR_LINUX}")
    set(JUCE_DIR_CANDIDATE "${JUCE_DIR_LINUX}")
elseif(APPLE AND EXISTS "/Applications/JUCE")
    set(JUCE_DIR_CANDIDATE "/Applications/JUCE")
elseif(WIN32 AND EXISTS "C:/Program Files/JUCE")
    set(JUCE_DIR_CANDIDATE "C:/Program Files/JUCE")
elseif(UNIX AND NOT APPLE AND EXISTS "/usr/local/JUCE")
    set(JUCE_DIR_CANDIDATE "/usr/local/JUCE")
endif()

if(JUCE_DIR_CANDIDATE STREQUAL "")
    message(FATAL_ERROR "JUCE not found. Set JUCE_DIR (e.g. export JUCE_DIR=/Applications/JUCE) or use -DJUCE_DIR=/path/to/JUCE")
endif()

set(JUCE_DIR "${JUCE_DIR_CANDIDATE}" CACHE PATH "Path to JUCE installation" FORCE)

if(NOT EXISTS "${JUCE_DIR}")
    message(FATAL_ERROR "JUCE not found at ${JUCE_DIR}. Check JUCE_DIR path.")
endif()

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
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:Matrix-Simulator,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:Matrix-Simulator,JUCE_VERSION>"
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

# =============================================================================
# COPY TO CENTRAL ARTEFACTS FOLDER
# When COPY_TO_ARTEFACTS_DIR is ON, copies the GUI app to a central custom folder.
# Structure: {ARTEFACTS_DIR}/{OS}/{arch}/Standalone/
# - macOS: ARM/, Intel/, Intel-Rosetta/, Universal/
# - Windows/Linux: no arch subfolder
# Gui-app build tree has no Standalone/ segment under *_artefacts/; destination uses Standalone/.
# =============================================================================

if(COPY_TO_ARTEFACTS_DIR)
    if(APPLE)
        set(ARTEFACTS_DIR_BASE "${ARTEFACTS_DIR_MACOS}")
    elseif(WIN32)
        set(ARTEFACTS_DIR_BASE "${ARTEFACTS_DIR_WINDOWS}")
    else()
        set(ARTEFACTS_DIR_BASE "${ARTEFACTS_DIR_LINUX}")
    endif()

    if(APPLE)
        set(ARCH_LIST ${CMAKE_OSX_ARCHITECTURES})
        list(FIND ARCH_LIST "arm64" IDX_ARM64)
        list(FIND ARCH_LIST "x86_64" IDX_X86_64)
        if(NOT IDX_ARM64 EQUAL -1 AND NOT IDX_X86_64 EQUAL -1)
            set(APP_ARCH_SUBFOLDER "Universal")
        elseif(NOT IDX_ARM64 EQUAL -1)
            set(APP_ARCH_SUBFOLDER "ARM")
        elseif(NOT IDX_X86_64 EQUAL -1)
            if(HOST_ARCH STREQUAL "arm64")
                set(APP_ARCH_SUBFOLDER "Intel-Rosetta")
            else()
                set(APP_ARCH_SUBFOLDER "Intel")
            endif()
        else()
            set(APP_ARCH_SUBFOLDER "ARM")
        endif()
        set(ARTEFACTS_DEST "${ARTEFACTS_DIR_BASE}/macOS/${APP_ARCH_SUBFOLDER}")
    elseif(WIN32)
        set(ARTEFACTS_DEST "${ARTEFACTS_DIR_BASE}/Windows")
    else()
        set(ARTEFACTS_DEST "${ARTEFACTS_DIR_BASE}/Linux")
    endif()

    message(STATUS "Build artefacts will be copied to: ${ARTEFACTS_DEST}")

    # POST_BUILD so Build: App / F5 also copy (not only a separate ALL target).
    if(APPLE)
        set(APP_SOURCE "${CMAKE_BINARY_DIR}/Matrix-Simulator_artefacts/$<CONFIG>/Matrix-Simulator.app")
        set(APP_DEST_DIR "${ARTEFACTS_DEST}/Standalone/Matrix-Simulator.app")
        add_custom_command(TARGET Matrix-Simulator POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Copying app to ${ARTEFACTS_DEST}/Standalone/"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${ARTEFACTS_DEST}/Standalone"
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${APP_DEST_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${APP_SOURCE}" "${APP_DEST_DIR}"
            COMMENT "Copying app artefact to ${ARTEFACTS_DEST}"
            VERBATIM
        )
    elseif(WIN32)
        set(APP_SOURCE "${CMAKE_BINARY_DIR}/Matrix-Simulator_artefacts/$<CONFIG>/Matrix-Simulator.exe")
        set(APP_DEST "${ARTEFACTS_DEST}/Standalone/Matrix-Simulator.exe")
        add_custom_command(TARGET Matrix-Simulator POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Copying app to ${ARTEFACTS_DEST}/Standalone/"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${ARTEFACTS_DEST}/Standalone"
            COMMAND ${CMAKE_COMMAND} -E rm -f "${APP_DEST}"
            COMMAND ${CMAKE_COMMAND} -E copy "${APP_SOURCE}" "${APP_DEST}"
            COMMENT "Copying app artefact to ${ARTEFACTS_DEST}"
            VERBATIM
        )
    else()
        set(APP_SOURCE "${CMAKE_BINARY_DIR}/Matrix-Simulator_artefacts/$<CONFIG>/Matrix-Simulator")
        set(APP_DEST "${ARTEFACTS_DEST}/Standalone/Matrix-Simulator")
        add_custom_command(TARGET Matrix-Simulator POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Copying app to ${ARTEFACTS_DEST}/Standalone/"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${ARTEFACTS_DEST}/Standalone"
            COMMAND ${CMAKE_COMMAND} -E rm -f "${APP_DEST}"
            COMMAND ${CMAKE_COMMAND} -E copy "${APP_SOURCE}" "${APP_DEST}"
            COMMENT "Copying app artefact to ${ARTEFACTS_DEST}"
            VERBATIM
        )
    endif()
endif()
