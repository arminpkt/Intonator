project(UnTETered VERSION 0.0.1)

if (APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

if (WIN32)
    set(_copy_after_build FALSE)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    set(_copy_after_build TRUE)
endif()

juce_add_plugin(UnTETered
        VERSION 0.0.1
        # ICON_BIG ...
        # ICON_SMALL ...
        COMPANY_NAME UnTETered
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT TRUE
        IS_MIDI_EFFECT FALSE
        # EDITOR_WANTS_KEYBOARD_FOCUS TRUE/FALSE
        COPY_PLUGIN_AFTER_BUILD ${_copy_after_build}
        PLUGIN_MANUFACTURER_CODE ACVV
        PLUGIN_CODE Into

        FORMATS VST3 Standalone
        PRODUCT_NAME "UnTETered")

juce_add_plugin(UnTETeredAU
        VERSION 0.0.1
        # ICON_BIG ...
        # ICON_SMALL ...
        COMPANY_NAME ACV
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT TRUE
        IS_MIDI_EFFECT TRUE
        # EDITOR_WANTS_KEYBOARD_FOCUS TRUE/FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
        PLUGIN_MANUFACTURER_CODE ACVV
        PLUGIN_CODE UnAU

        FORMATS AU
        PRODUCT_NAME "UnTETered")

option(UNTETERED_DRM "Build with Moonbase licensing" ON)

if(UNTETERED_DRM)
    if(APPLE)
        enable_language(OBJC OBJCXX)   # module compiles an Obj-C++ file on macOS
    endif()

    CPMAddPackage(
            NAME moonbase-cpp
            GITHUB_REPOSITORY Moonbase-sh/moonbase-cpp
            GIT_TAG v4.13.0
            DOWNLOAD_ONLY YES)             # don't add the root CMake (OpenSSL/CURL)

    juce_add_module(${moonbase-cpp_SOURCE_DIR}/modules/moonbase_licensing)

    target_link_libraries(UnTETered PRIVATE moonbase_licensing)
    target_compile_definitions(UnTETered PRIVATE JUCE_USE_CURL=0)
    target_link_libraries(UnTETeredAU PRIVATE moonbase_licensing)
    target_compile_definitions(UnTETeredAU PRIVATE JUCE_USE_CURL=0)
else()
    message(WARNING "UnTETered is being built WITHOUT DRM")
endif()

target_compile_definitions(UnTETered PRIVATE
        UNTETERED_DRM=$<BOOL:${UNTETERED_DRM}>)
target_compile_definitions(UnTETeredAU PRIVATE
        UNTETERED_DRM=$<BOOL:${UNTETERED_DRM}>)

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
        DAWbackends/AbletonBackend.cpp
        DAWbackends/LogicBackend.cpp
        DRM/LicenseGuard.cpp
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
        juce::juce_osc
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

target_sources(UnTETeredAU
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
        DAWbackends/AbletonBackend.cpp
        DAWbackends/LogicBackend.cpp
        DRM/LicenseGuard.cpp
)

target_compile_definitions(UnTETeredAU
        PUBLIC
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(UnTETeredAU
        PRIVATE
        juce::juce_audio_utils
        juce::juce_animation
        juce::juce_osc
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# ---- Ableton Remote Script (UnTETeredSync) ---------------------------------
option(UNTETERED_COPY_REMOTE_SCRIPT
        "Copy the UnTETeredSync Remote Script into Ableton's User Library when building" ON)

if (UNTETERED_COPY_REMOTE_SCRIPT)
    # Where the .py files live in your repo -- adjust if they are somewhere else
    set(UNTETERED_REMOTE_SCRIPT_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/scripts/ableton/UnTETered"
            CACHE PATH "Folder containing the UnTETeredSync .py files")

    if (APPLE)
        set(_default_user_library "$ENV{HOME}/Music/Ableton/User Library")
    elseif (WIN32)
        # "Personal" = the real Documents folder, even when redirected to OneDrive
        cmake_host_system_information(RESULT _docs
                QUERY WINDOWS_REGISTRY
                "HKCU/Software/Microsoft/Windows/CurrentVersion/Explorer/User Shell Folders"
                VALUE "Personal"
                ERROR_VARIABLE _docs_err)
        if (NOT _docs OR _docs_err)
            set(_docs "$ENV{USERPROFILE}/Documents")
        endif()
        file(TO_CMAKE_PATH "${_docs}/Ableton/User Library" _default_user_library)
    endif()
    set(ABLETON_USER_LIBRARY "${_default_user_library}"
            CACHE PATH "Ableton Live User Library folder")

    set(_remote_script_dest "${ABLETON_USER_LIBRARY}/Remote Scripts/UnTETered")
    file(GLOB _remote_script_files CONFIGURE_DEPENDS "${UNTETERED_REMOTE_SCRIPT_SOURCE}/*.py")

    if (_remote_script_files AND ABLETON_USER_LIBRARY)
        set(_stamp "${CMAKE_CURRENT_BINARY_DIR}/UnTETeredSync_copied.stamp")

        add_custom_command(
                OUTPUT  "${_stamp}"
                COMMAND ${CMAKE_COMMAND} -E make_directory "${_remote_script_dest}"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_remote_script_files} "${_remote_script_dest}"
                COMMAND ${CMAKE_COMMAND} -E touch "${_stamp}"
                DEPENDS ${_remote_script_files}
                COMMENT "Copying UnTETered Remote Script to ${_remote_script_dest}"
                VERBATIM)

        add_custom_target(UnTETeredSyncRemoteScript DEPENDS "${_stamp}")
        if (APPLE)
            add_dependencies(UnTETeredAU UnTETeredSyncRemoteScript)
        endif()
        add_dependencies(UnTETeredAU UnTETeredSyncRemoteScript)
    else()
        message(WARNING "UnTETeredSync Remote Script not copied: no .py files found in "
                "${UNTETERED_REMOTE_SCRIPT_SOURCE} or no Ableton User Library set")
    endif()
endif()