cmake_minimum_required(VERSION 3.15)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Minimum OS X deployment target")
project(ChowKick VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
add_subdirectory(modules)

# juce_set_vst2_sdk_path(C:/SDKs/VST_SDK/VST2_SDK/)
set(JUCE_FORMATS AU VST3 Standalone)

# Build LV2 only on Linux
if(UNIX AND NOT APPLE)
    message(STATUS "Building LV2 plugin format")
    list(APPEND JUCE_FORMATS LV2)
endif()

# Build VST2 is SDK set
if(TARGET juce_vst2_sdk)
    message(STATUS "Building VST2 plugin format")
    list(APPEND JUCE_FORMATS VST)
endif()

juce_add_plugin(ChowKick
    COMPANY_NAME chowdsp
    PLUGIN_MANUFACTURER_CODE Chow
    PLUGIN_CODE Kik5
    IS_SYNTH True
    NEEDS_MIDI_INPUT True
    FORMATS ${JUCE_FORMATS}
    ProductName "ChowKick"
)

juce_generate_juce_header(ChowKick)
add_subdirectory(src)
include_directories(src)
add_subdirectory(res)

target_compile_definitions(ChowKick PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(ChowKick PRIVATE juce_plugin_modules)
