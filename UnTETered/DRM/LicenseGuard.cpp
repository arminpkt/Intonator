#include "LicenseGuard.h"

#ifndef UNTETERED_DRM
 #error "Define UNTETERED_DRM=0 or 1 on the plugin target"
#endif

#if ! UNTETERED_DRM && defined (NDEBUG) && ! defined (UNTETERED_ALLOW_NODRM_RELEASE)
 #error "No-DRM build in Release config. Define UNTETERED_ALLOW_NODRM_RELEASE if intended."
#endif

#if UNTETERED_DRM
 #include <moonbase_licensing/moonbase_licensing.h>   // pulls in Apple headers, only here
 #include "MoonbaseConfig.h"
#endif

namespace unTETered
{

#if UNTETERED_DRM
struct LicenseGuard::Impl
{
    static moonbase::juce_integration::ActivationConfig makeConfig()
    {
        moonbase::juce_integration::ActivationConfig c;
        c.endpoint  = std::string(moonbase_keys::endpoint);
        c.productId = std::string(moonbase_keys::productId);
        c.publicKey = std::string(moonbase_keys::publicKey);
        c.trialLengthDays = 90;

        c.onDiagnostic = [](const juce::String& msg)
        {
            juce::Logger::writeToLog("[license] " + msg);
        };
        return c;
    }

    moonbase::juce_integration::ActivationController controller { makeConfig() };
};
#else
struct LicenseGuard::Impl {};
#endif

LicenseGuard::LicenseGuard()
    : impl(std::make_unique<Impl>())
{
   #if UNTETERED_DRM
    impl->controller.start();   // loads any stored license
   #endif
}

LicenseGuard::~LicenseGuard() = default;   // here, because Impl is complete here

bool LicenseGuard::isLicensed() noexcept
{
   #if UNTETERED_DRM
    return impl->controller.licensedFlag().load();
   #else
    return true;
   #endif
}

std::unique_ptr<juce::Component> LicenseGuard::createActivationUI(std::function<void (bool)> onChanged)
{
   #if UNTETERED_DRM
    using namespace moonbase::juce_integration;
    auto ui = std::make_unique<ActivationComponent>(impl->controller);   // non-owning
    auto* raw = ui.get();
    ui->onClose = [raw] { raw->setVisible(false); };
    ui->onActivationChanged = [raw, onChanged](bool activated)
    {
        if (activated) raw->setVisible(false);
        if (onChanged) onChanged(activated);
    };
    return ui;
   #else
    juce::ignoreUnused(onChanged);
    return nullptr;
   #endif
}

} // namespace unTETered