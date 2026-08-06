#pragma once

#include <ucf/Services/ClientInfoService/ClientInfoTypes.h>
#include <ucf/Services/ClientInfoService/IClientInfoServiceCallback.h>

namespace ucf::service {

// Internal sink: ClientInfoModel invokes it after a state change; ClientInfoService
// implements it and translates each event into the corresponding outward
// IClientInfoServiceCallback notification. Defining a separate sink (rather than
// having the Model call NotificationHelper directly) keeps the Model free of
// service-framework dependencies and makes it trivial to add new events:
// just add a method here + matching override in ClientInfoService.
class IClientInfoNotificationSink
{
public:
    virtual ~IClientInfoNotificationSink() = default;

    // Fired exactly once after language/theme have been populated from the DB.
    virtual void onClientInfoReady() {}

    // Fired exactly once if the initial DB-driven load could not complete.
    virtual void onClientInfoLoadFailed(ClientInfoLoadError /*error*/) {}

    // Fired on every language change (initial load + setLanguage calls).
    virtual void onClientLanguageChanged(model::LanguageType /*languageType*/) {}

    // Fired on every theme change (initial load + setTheme calls).
    virtual void onClientThemeChanged(model::ThemeType /*themeType*/) {}
};

} // namespace ucf::service
