//
// Created by Armin Peukert on 20.10.26.
//

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include <memory>

namespace unTETered
{

class LicenseGuard
{
public:
    LicenseGuard();
    ~LicenseGuard();

    /** Lock-free, safe to call from the audio thread. */
    bool isLicensed() noexcept;

    /** Overlay for the editor, or nullptr in no-DRM builds. */
    std::unique_ptr<juce::Component> createActivationUI (std::function<void (bool)> onChanged = {});

private:
   struct Impl;
    std::unique_ptr<Impl> impl;

    JUCE_DECLARE_NON_COPYABLE (LicenseGuard)
};

} // namespace unTETered
