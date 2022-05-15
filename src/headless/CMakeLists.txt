juce_add_console_app(ChowKick_headless
    PRODUCT_NAME "ChowKick"
    COMPANY_NAME chowdsp
)

juce_generate_juce_header(ChowKick_headless)
add_custom_command(TARGET ChowKick_headless
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E echo "copying $<TARGET_FILE:ChowKick_headless> to ${PROJECT_BINARY_DIR}/ChowKick"
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ChowKick_headless> ${PROJECT_BINARY_DIR}/ChowKick)

target_sources(ChowKick_headless PRIVATE
    main.cpp
    ScreenshotGenerator.cpp
)

target_precompile_headers(ChowKick_headless PRIVATE ../pch.h)
target_include_directories(ChowKick_headless
    PRIVATE
        ../
        ../../modules
        ../../modules/JUCE/modules
        ../../modules/xsimd/include
        ../../modules/chowdsp_utils/modules/common
        ../../modules/chowdsp_utils/modules/dsp
        ../../modules/chowdsp_utils/modules/plugin
        ../../modules/chowdsp_utils/modules/gui
        ../../modules/tuning-library/include
        ../../modules/chowdsp_wdf/include
)

target_link_libraries(ChowKick_headless PUBLIC
    BinaryData
    ChowKick
)

set_target_properties(ChowKick_headless PROPERTIES CXX_VISIBILITY_PRESET hidden)
