set(binary_data_files
    gui.xml
    gui_ios.xml

    presets/Bouncy.chowpreset
    presets/Default.chowpreset
    presets/Tight.chowpreset
    presets/Tonal.chowpreset
    presets/Wonky_Synth.chowpreset
)

if(IOS)
    set(tuning_lib_zip "${CMAKE_CURRENT_BINARY_DIR}/tuning_library.zip")
    message(STATUS "Adding tuning library to BinaryData (${tuning_lib_zip})")
    create_zip(${tuning_lib_zip} "${CMAKE_CURRENT_SOURCE_DIR}/tuning_library" "${CMAKE_CURRENT_SOURCE_DIR}")
    list(APPEND binary_data_files ${tuning_lib_zip})
endif()
    
juce_add_binary_data(BinaryData SOURCES ${binary_data_files})

# Need to build BinaryData with -fPIC flag on Linux
set_target_properties(BinaryData PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE)
