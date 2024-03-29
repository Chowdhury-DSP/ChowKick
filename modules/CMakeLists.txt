add_subdirectory(JUCE)
add_subdirectory(chowdsp_utils)

subproject_version(JUCE juce_version)
message(STATUS "VERSION for JUCE: ${juce_version}")

subproject_version(chowdsp_utils chowdsp_version)
message(STATUS "VERSION for chowdsp_utils: ${chowdsp_version}")

juce_add_modules(foleys_gui_magic)
add_subdirectory(chowdsp_wdf)

add_subdirectory(tuning-library EXCLUDE_FROM_ALL)

include(cmake/WarningFlags.cmake)
add_library(juce_plugin_modules STATIC)
target_link_libraries(juce_plugin_modules
    PRIVATE
        BinaryData
        juce::juce_audio_utils
        juce::juce_audio_plugin_client
        foleys_gui_magic
        chowdsp::chowdsp_filters
        chowdsp::chowdsp_sources
        chowdsp::chowdsp_foleys
        chowdsp::chowdsp_plugin_base
        chowdsp::chowdsp_plugin_utils
        chowdsp::chowdsp_presets
        chowdsp::chowdsp_wdf
        tuning-library
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        warning_flags
)

target_compile_definitions(juce_plugin_modules
    PUBLIC
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_JACK=1
        JUCE_ALSA=1
        JucePlugin_Manufacturer="chowdsp"
        JucePlugin_VersionString="${CMAKE_PROJECT_VERSION}"
        JucePlugin_Name="${CMAKE_PROJECT_NAME}"
        FOLEYS_SHOW_GUI_EDITOR_PALLETTE=0
        FOLEYS_ENABLE_OPEN_GL_CONTEXT=1
        JUCE_MODAL_LOOPS_PERMITTED=1
        FOLEYS_ENABLE_BINARY_DATA=1
    INTERFACE
        $<TARGET_PROPERTY:juce_plugin_modules,COMPILE_DEFINITIONS>
)

target_include_directories(juce_plugin_modules
    PUBLIC
        tuning-library/include
    INTERFACE
        $<TARGET_PROPERTY:juce_plugin_modules,INCLUDE_DIRECTORIES>
)

if(CHOWKICK_BUILD_CLAP)
    add_subdirectory(clap-juce-extensions EXCLUDE_FROM_ALL)
    target_link_libraries(juce_plugin_modules
        PRIVATE
            chowdsp::chowdsp_clap_extensions
            clap_juce_extensions
    )
endif()

if (CHOWKICK_BUILD_WITH_MTS)
    message(STATUS "Linking with MTS")
    add_subdirectory(oddsound-mts EXCLUDE_FROM_ALL)
    target_link_libraries(juce_plugin_modules PRIVATE oddsound-mts)
    target_compile_definitions(juce_plugin_modules PUBLIC CHOW_KICK_WITH_MTS=1)
endif()

set_target_properties(juce_plugin_modules PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE
    VISIBILITY_INLINES_HIDDEN TRUE
    C_VISBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
)

if(ASIOSDK_DIR)
    message(STATUS "Using ASIO SDK from ${ASIOSDK_DIR}")
    target_include_directories(juce_plugin_modules PUBLIC ${ASIOSDK_DIR}/common)
    target_compile_definitions(juce_plugin_modules PUBLIC JUCE_ASIO=1)
endif()

if(IOS)
    include(cmake/CreateZip.cmake)
    target_link_libraries(juce_plugin_modules PRIVATE juce::juce_product_unlocking)
    target_compile_definitions(juce_plugin_modules PUBLIC JUCE_IN_APP_PURCHASES=1)
endif()

# Enable OpenGL on Windows by default.
if (WIN32)
    option(CHOWDSP_ENABLE_OPENGL "Enable OpenGL for this plugin" ON)
else()
    option(CHOWDSP_ENABLE_OPENGL "Enable OpenGL for this plugin" OFF)
endif()

if(CHOWDSP_ENABLE_OPENGL)
    message(STATUS "Linking with OpenGL")
    target_link_libraries(juce_plugin_modules PRIVATE juce::juce_opengl)
    target_compile_definitions(juce_plugin_modules PUBLIC FOLEYS_ENABLE_OPEN_GL_CONTEXT=1)
endif()
