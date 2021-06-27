juce_add_binary_data(BinaryData SOURCES
    gui.xml
    gui_ios.xml
    LeftArrow.svg
    RightArrow.svg

    presets/Bouncy.chowpreset
    presets/Default.chowpreset
    presets/Tight.chowpreset
    presets/Tonal.chowpreset
    presets/Wonky_Synth.chowpreset
)

# Need to build BinaryData with -fPIC flag on Linux
set_target_properties(BinaryData PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE)
