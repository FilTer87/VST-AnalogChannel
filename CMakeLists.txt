cmake_minimum_required(VERSION 3.15)

project(AnalogChannel VERSION 0.5.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(ANALOGCHANNEL_STANDALONE "Build the standalone target alongside VST3" ON)

#------------------------------------------------------------------------------
# JUCE dependency
#------------------------------------------------------------------------------
# Prefer a system-provided JUCE package (juce-config) when available.
# If JUCE_DIR is provided, use the source checkout directly.
if(DEFINED JUCE_DIR)
    add_subdirectory(${JUCE_DIR} JUCE)
else()
    find_package(JUCE CONFIG)
    if(NOT JUCE_FOUND)
        message(FATAL_ERROR
            "JUCE was not found. Set JUCE_DIR to a JUCE 7 checkout (or install "
            "the juce-config package).")
    endif()
endif()

#------------------------------------------------------------------------------
# Resources embedded as BinaryData (matches the .jucer resources)
#------------------------------------------------------------------------------
juce_add_binary_data(AnalogChannelResources
    SOURCES
        Docs/images/logos/favicon-32x32.png
        Docs/images/logos/logo_banner.png
)

set(ANALOGCHANNEL_FORMATS VST3)
if(ANALOGCHANNEL_STANDALONE)
    list(APPEND ANALOGCHANNEL_FORMATS Standalone)
endif()

#------------------------------------------------------------------------------
# Plugin target
#------------------------------------------------------------------------------
juce_add_plugin(AnalogChannel
    COMPANY_NAME "KuramaSound"
    COMPANY_EMAIL "filippo@kuramasound.com"
    PLUGIN_MANUFACTURER_CODE KurS
    PLUGIN_CODE AnCh
    FORMATS ${ANALOGCHANNEL_FORMATS}
    PRODUCT_NAME "AnalogChannel"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    VST3_CATEGORIES "Distortion;Dynamics;EQ;Filter;Fx;Stereo;Tools"
    COPY_PLUGIN_AFTER_BUILD TRUE
    VST3_COPY_DIR "${CMAKE_BINARY_DIR}/AnalogChannel_artefacts/VST3"
    VERSION ${PROJECT_VERSION}
)

juce_generate_juce_header(AnalogChannel)

target_sources(AnalogChannel PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/GUI/Common/PluginHeaderBar.cpp
    Source/GUI/Common/PresetBarComponent.cpp
)

target_include_directories(AnalogChannel PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Algorithms
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/GUI
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Sections
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

target_compile_definitions(AnalogChannel
    PRIVATE
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
        JUCE_STRICT_REFCOUNTEDPOINTER=1
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

target_link_libraries(AnalogChannel
    PRIVATE
        AnalogChannelResources
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
)

# Organize sources in IDEs
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Source FILES
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/GUI/Common/PluginHeaderBar.cpp
    Source/GUI/Common/PresetBarComponent.cpp
)

#------------------------------------------------------------------------------
# Installation helpers (Linux)
#------------------------------------------------------------------------------
include(GNUInstallDirs)

set(ANALOGCHANNEL_VST3_DIR "${CMAKE_CURRENT_BINARY_DIR}/AnalogChannel_artefacts/VST3")
install(DIRECTORY "${ANALOGCHANNEL_VST3_DIR}/AnalogChannel.vst3"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/vst3"
        OPTIONAL)

if(ANALOGCHANNEL_STANDALONE)
    install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/AnalogChannel_artefacts/Standalone/AnalogChannel"
            DESTINATION "${CMAKE_INSTALL_BINDIR}"
            OPTIONAL)
endif()


