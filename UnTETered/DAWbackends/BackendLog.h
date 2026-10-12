#pragma once
#include <juce_core/juce_core.h>

inline void backendLog (const juce::String& message)
{
    static std::unique_ptr<juce::FileLogger> logger(
        juce::FileLogger::createDefaultAppLogger("UnTETered", "UnTETered.log", "UnTETered log started"));

    if (logger == nullptr)
        return;

    const auto now = juce::Time::getCurrentTime();
    logger->logMessage (now.formatted ("%H:%M:%S.")
                        + juce::String (now.getMilliseconds()).paddedLeft ('0', 3)
                        + "  " + message);
}

// Same usage as DBG: BACKEND_LOG ("value: " << x);
#define BACKEND_LOG(textToWrite) \
    do { juce::String backendLogBuffer; backendLogBuffer << textToWrite; backendLog (backendLogBuffer); } while (false)