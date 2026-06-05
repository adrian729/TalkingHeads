cmake_minimum_required(VERSION 3.22)
project(TalkingHeads VERSION 1.0.0 LANGUAGES C CXX)

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(JUCE)

juce_add_plugin(TalkingHeads
    PRODUCT_NAME            "TalkingHeads"
    COMPANY_NAME            "Brutus729"
    BUNDLE_ID               "com.brutus729.TalkingHeads"
    PLUGIN_MANUFACTURER_CODE Brut
    PLUGIN_CODE              Thed
    FORMATS                  VST3 AU Standalone
    VST3_CATEGORIES          Fx EQ Modulation
    AU_MAIN_TYPE             kAudioUnitType_Effect
    IS_SYNTH                 FALSE
    NEEDS_MIDI_INPUT         FALSE
    NEEDS_MIDI_OUTPUT        FALSE
    COPY_PLUGIN_AFTER_BUILD  TRUE
    VST3_CAN_REPLACE_VST2    FALSE
)

# Generates the umbrella <JuceHeader.h> the sources include
juce_generate_juce_header(TalkingHeads)

target_sources(TalkingHeads PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/PluginStateManager.cpp
    Source/ParameterObject.cpp
    Source/MultiBandEQ.cpp
    Source/EQBand.cpp
    Source/MultiBandCompressor.cpp
    Source/CompressorBand.cpp
    Source/Imager.cpp
)

target_compile_definitions(TalkingHeads PUBLIC
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)

target_link_libraries(TalkingHeads
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
