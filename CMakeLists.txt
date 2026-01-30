cmake_minimum_required(VERSION 3.22)

project(Dynoverb VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find JUCE - adjust this path to your JUCE installation
# Option 1: If JUCE is installed via CMake
find_package(JUCE CONFIG QUIET)

# Option 2: If JUCE is in a sibling directory or specified location
if(NOT JUCE_FOUND)
    if(DEFINED ENV{JUCE_DIR})
        set(JUCE_DIR $ENV{JUCE_DIR})
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE")
        set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE")
    elseif(EXISTS "/usr/local/share/JUCE")
        set(JUCE_DIR "/usr/local/share/JUCE")
    elseif(EXISTS "$ENV{HOME}/JUCE")
        set(JUCE_DIR "$ENV{HOME}/JUCE")
    endif()

    if(DEFINED JUCE_DIR)
        add_subdirectory(${JUCE_DIR} ${CMAKE_BINARY_DIR}/JUCE)
    else()
        message(FATAL_ERROR "JUCE not found. Please set JUCE_DIR or install JUCE.")
    endif()
endif()

# Plugin configuration
juce_add_plugin(Dynoverb
    COMPANY_NAME "Ian Fletcher Audio"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE IFaA
    PLUGIN_CODE DynV
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Dynoverb"
)

# Source files
target_sources(Dynoverb
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/DSP/AlgorithmicReverb.cpp
        Source/DSP/ShimmerReverb.cpp
        Source/DSP/SpringReverb.cpp
        Source/DSP/GatedReverb.cpp
)

# Header search paths
target_include_directories(Dynoverb
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/DSP
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/UI
        ${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode
)

# Preprocessor definitions
target_compile_definitions(Dynoverb
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

# Link JUCE modules
target_link_libraries(Dynoverb
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
