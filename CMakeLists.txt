cmake_minimum_required(VERSION 3.22)
project(GBASynth VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 7.0.12 GIT_SHALLOW ON)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(GBASynth
    COMPANY_NAME "GBA"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    PLUGIN_MANUFACTURER_CODE Gbas
    PLUGIN_CODE Gba1
    FORMATS VST3 Standalone
    PRODUCT_NAME "GBA Synth"
    COPY_PLUGIN_AFTER_BUILD TRUE)

target_sources(GBASynth PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(GBASynth PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(GBASynth
    PRIVATE juce::juce_audio_utils juce::juce_audio_processors
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags)
