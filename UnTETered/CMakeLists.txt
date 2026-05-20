project(UnTETered VERSION 0.0.1)

if (APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

juce_add_plugin(UnTETered
        VERSION 0.0.1
        # ICON_BIG ...
        # ICON_SMALL ...
        COMPANY_NAME ACV
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT TRUE
        IS_MIDI_EFFECT FALSE
        # EDITOR_WANTS_KEYBOARD_FOCUS TRUE/FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
        PLUGIN_MANUFACTURER_CODE ACVV
        PLUGIN_CODE Into

        FORMATS AU VST3 Standalone
        PRODUCT_NAME "UnTETered")

target_sources(UnTETered
        PRIVATE
        PluginEditor.cpp
        PluginProcessor.cpp
        view/interfaces/Grid2D.cpp
        view/interfaces/TextInput.cpp
        view/interfaces/GridStateSerialiser.cpp
        view/interfaces/PianoRollStateSerialiser.cpp
        view/interfaces/PianoRollStateHelpers.cpp
        logic/util.cpp
        logic/noteRegion.cpp
        view/interfaces/PianoRoll.cpp
        view/interfaces/PianoRollSettingsBar.cpp
)

target_compile_definitions(UnTETered
        PUBLIC
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(UnTETered
        PRIVATE
        juce::juce_audio_utils
        juce::juce_animation
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)