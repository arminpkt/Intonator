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
        logic/util.cpp
        PianoRoll.cpp)

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

#set(VST3_DESTINATION "/Library/Audio/Plug-Ins/VST3/")
#set(AU_DESTINATION "/Library/Audio/Plug-Ins/Components/")

# Copy VST3 and AU files upon build completion
#add_custom_command(TARGET UnTETered POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy
#        "$<TARGET_FILE_DIR:UnTETered>/UnTETered.vst3"
#        "${VST3_DESTINATION}/UnTETered.vst3"
#        COMMAND ${CMAKE_COMMAND} -E copy
#        "$<TARGET_FILE_DIR:UnTETered>/UnTETered.component"
#        "${AU_DESTINATION}/UnTETered.component"
#)