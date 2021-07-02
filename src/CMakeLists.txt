if(NOT (IOS OR MACOS_RELEASE))
    add_subdirectory(headless)
endif()

target_sources(ChowKick PRIVATE
    ChowKick.cpp

    dsp/OutputFilter.cpp
    dsp/PulseShaper.cpp
    dsp/ResonantFilter.cpp
    dsp/Trigger.cpp

    gui/DisabledSlider.cpp
    gui/FilterViewer.cpp
    gui/PulseViewer.cpp

    presets/PresetComp.cpp
    presets/PresetManager.cpp
)

if(IOS)
    target_sources(ChowKick PRIVATE
        gui/TipJar.cpp
    )
endif()

target_precompile_headers(ChowKick PRIVATE pch.h)
    