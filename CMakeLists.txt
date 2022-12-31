cmake_minimum_required(VERSION 3.15)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Minimum OS X deployment target")
project(ChowKick VERSION 1.1.1)

set(CMAKE_CXX_STANDARD 17)

# juce_set_vst2_sdk_path(C:/SDKs/VST_SDK/VST2_SDK)
# set default plugin formats to build
if(IOS)
    set(JUCE_FORMATS Standalone AUv3)
    option(CHOWKICK_BUILD_CLAP "Create a CLAP target for ChowKick" OFF)
else()
    set(JUCE_FORMATS AU VST3 Standalone)
    option(CHOWKICK_BUILD_CLAP "Create a CLAP target for ChowKick" ON)
endif()

if (CHOWKICK_BUILD_CLAP AND (CMAKE_VERSION VERSION_LESS 3.21.0))
    message(WARNING "CLAP builds require CMake version 3.21 or higher! To disable CLAP builds, run CMake configuration with \"-DCHOWKICK_BUILD_CLAP=OFF\"")
endif()

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

add_subdirectory(modules)

juce_add_plugin(ChowKick
    COMPANY_NAME chowdsp
    PLUGIN_MANUFACTURER_CODE Chow
    PLUGIN_CODE Kik5
    IS_SYNTH True
    NEEDS_MIDI_INPUT True
    FORMATS ${JUCE_FORMATS}
    ProductName "ChowKick"
    ICON_BIG res/logo.png
    LV2_URI http://github.com/Chowdhury-DSP/ChowKick
    NEEDS_STORE_KIT TRUE
    REQUIRES_FULL_SCREEN TRUE
    IPHONE_SCREEN_ORIENTATIONS UIInterfaceOrientationLandscapeLeft UIInterfaceOrientationLandscapeRight
    IPAD_SCREEN_ORIENTATIONS UIInterfaceOrientationLandscapeLeft UIInterfaceOrientationLandscapeRight
)

if(CHOWKICK_BUILD_CLAP)
    clap_juce_extensions_plugin(
        TARGET ChowKick
        CLAP_ID "org.chowdsp.ChowKick"
        CLAP_FEATURES instrument drum
        CLAP_PROCESS_EVENTS_RESOLUTION_SAMPLES 64
        CLAP_USE_JUCE_PARAMETER_RANGES DISCRETE
    )
endif()

juce_generate_juce_header(ChowKick)
add_subdirectory(src)
include_directories(src)
add_subdirectory(res)

target_compile_definitions(ChowKick PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(ChowKick PRIVATE juce_plugin_modules)

# we need these flags for notarization on MacOS
option(MACOS_RELEASE "Set build flags for MacOS Release" OFF)
if(MACOS_RELEASE)
    message(STATUS "Setting MacOS release flags...")
    set_target_properties(ChowKick_Standalone PROPERTIES
        XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES)
endif()

if(IOS)
    message(STATUS "Setting iOS-specific properties...")

    foreach(target IN ITEMS BinaryData juce_plugin_modules ChowKick ChowKick_Standalone ChowKick_AUv3)
        set_target_properties(${target}
            PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "./"
                ARCHIVE_OUTPUT_DIRECTORY "./"
                LIBRARY_OUTPUT_DIRECTORY "./")
    endforeach()

    set_target_properties(ChowKick_Standalone PROPERTIES
            XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)"
            XCODE_ATTRIBUTE_SKIP_INSTALL "NO"
            XCODE_ATTRIBUTE_ENABLE_IN_APP_PURCHASE "YES")

    set_target_properties(ChowKick_AUv3 PROPERTIES
            XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)/ChowKick.app/PlugIns"
            XCODE_ATTRIBUTE_SKIP_INSTALL "NO"
            XCODE_ATTRIBUTE_ENABLE_IN_APP_PURCHASE "YES")
endif()
