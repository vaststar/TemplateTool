#pragma once

#include <ucf/services/ClientInfoService/ClientInfoTypes.h>
#include <ucf/services/ClientInfoService/IClientInfoServiceCallback.h>

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

    // Reports a failed initialization request or DB-driven load; explicit retries may report again.
    virtual void onClientInfoLoadFailed(ClientInfoLoadError /*error*/) {}

    // Fired on every language change (initial load + setLanguage calls).
    virtual void onClientLanguageChanged(model::LanguageType /*languageType*/) {}

    // Fired on every theme change (initial load + setTheme calls).
    virtual void onClientThemeChanged(model::ThemeType /*themeType*/) {}
};

} // namespace ucf::service
