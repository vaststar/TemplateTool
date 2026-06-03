#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>

#include <ucf/Services/ClientInfoService/ClientInfoTypes.h>

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service {

// Database I/O only for the shared client settings table (Language / Theme).
// Holds no cached state and performs no fallback / default logic; the Model
// decides what to do when a row is absent (std::nullopt) or when the DB cannot
// be reached (callback never invoked → caller times out via its own mechanism).
class ClientInfoDBAccess
{
public:
    using LoadLanguageCallback = std::function<void(std::optional<model::LanguageType>)>;
    using LoadThemeCallback    = std::function<void(std::optional<model::ThemeType>)>;

    explicit ClientInfoDBAccess(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ~ClientInfoDBAccess() = default;
    ClientInfoDBAccess(const ClientInfoDBAccess&) = delete;
    ClientInfoDBAccess(ClientInfoDBAccess&&) = delete;
    ClientInfoDBAccess& operator=(const ClientInfoDBAccess&) = delete;
    ClientInfoDBAccess& operator=(ClientInfoDBAccess&&) = delete;

public:
    void               setDatabaseId(const std::string& dbId);
    const std::string& getDatabaseId() const;

    // Returns true iff the fetch was dispatched (DBService + dbId both available).
    // On false, the callback is NOT invoked and the caller should treat as failure.
    bool loadLanguage(LoadLanguageCallback callback) const;
    bool loadTheme   (LoadThemeCallback    callback) const;

    // Best-effort persist; silently no-ops if DBService is gone.
    void saveLanguage(model::LanguageType languageType) const;
    void saveTheme   (model::ThemeType    themeType)    const;

private:
    const ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
    std::string                              mDatabaseId;
};

} // namespace ucf::service
