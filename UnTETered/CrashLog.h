//
// Created by Vos de Mens on 08/10/2026.
//

#pragma once

namespace CrashLog {
    inline juce::String indentation = "";

    inline juce::File getLogFile() {
        return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("UnTETered")
            .getChildFile("log.txt");
    }

    inline void log(const juce::String& message, bool indent = true) {
        auto logFile = getLogFile();
        auto result = logFile.getParentDirectory().createDirectory();
        if (result.failed())
            DBG("failed to create log file");
        logFile.appendText("[" + juce::Time::getCurrentTime().toString(true, true) + "] " +
            (indent ? indentation : "") + message + "\n");
    }

    inline void logCrash() {
        log("=== CRASH ===\n" + juce::SystemStats::getStackBacktrace() + "\n\n", false);
    }

    inline void flushLogIfBig() {
        auto logFile = getLogFile();
        if (logFile.getSize() > 1000000) {
            auto result = logFile.deleteFile();
            if (result)
                log("Flushed log", false);
            else
                DBG("Flushing log file failed");
        }
    }

    struct ScopeLog {
        explicit ScopeLog(const char* name) {
            log( juce::String("> ") + name);
            indentation += "    ";
        }

        ~ScopeLog() {
            indentation = indentation.dropLastCharacters(4);
            log("<");
        }

        // non-copyable
        ScopeLog(const ScopeLog&) = delete;
        ScopeLog& operator=(const ScopeLog&) = delete;
    };

    #define LOG_SCOPE() ScopeLog _scopeLog(__FUNCTION__)
}

