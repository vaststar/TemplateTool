#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>

#include <ucf/Services/ClientInfoService/ClientInfoTypes.h>
#include <ucf/Utilities/SinkNotifier/SinkNotifier.h>

#include "ClientInfoNotificationSink.h"

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service {

class ClientInfoDBAccess;

// In-memory client settings (language, theme) coordinated with database
// persistence. Owns ClientInfoDBAccess for all DB I/O so the rest of the service
// never sees IDataWarehouseService directly. Drives a small state machine that
// fires onClientInfoReady / onClientInfoLoadFailed exactly once via the sink.
class ClientInfoModel : public ucf::utilities::SinkNotifier<IClientInfoNotificationSink>
{
public:
    explicit ClientInfoModel(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ~ClientInfoModel();
    ClientInfoModel(const ClientInfoModel&) = delete;
    ClientInfoModel(ClientInfoModel&&) = delete;
    ClientInfoModel& operator=(const ClientInfoModel&) = delete;
    ClientInfoModel& operator=(ClientInfoModel&&) = delete;

public:
    // ===== Read =====
    model::LanguageType getLanguage() const;
    model::ThemeType    getTheme()    const;
    bool                isReady()     const;

    // ===== Write (memory → DB → sink) =====
    void setLanguage(model::LanguageType languageType);
    void setTheme   (model::ThemeType    themeType);

    // ===== Lifecycle =====
    // Called when DataWarehouseService reports the shared DB ready. Only binds the
    // DB id (Uninit → DbBound) and does NOT trigger any load. Callers must invoke
    // loadSettings() explicitly. If loadSettings() was called before bindDatabase(),
    // the parked request is auto-promoted here.
    void bindDatabase(const std::string& dbId);

    // Explicit load entry point. Safe to call in any order relative to bindDatabase():
    // if the DB is not yet bound the request is parked and replayed once bindDatabase()
    // arrives. Once DbBound the call transitions to Loading and dispatches the two
    // async queries; finishLoadSuccess / finishLoadFailure fires exactly once.
    void loadSettings();

private:
    void finishLoadSuccess();
    void finishLoadFailure(ClientInfoLoadError error);

    enum class LoadStage : std::uint8_t {
        Uninit,    // bindDatabase has not been called.
        DbBound,   // bindDatabase done; load not started.
        Loading,   // Async DB queries in flight.
        Ready,     // Both chunks succeeded.
        Failed,    // Load failed (retryable).
    };

private:
    const std::unique_ptr<ClientInfoDBAccess> mDBAccess;

    std::atomic<model::LanguageType>          mLanguage{model::LanguageType::CHINESE_SIMPLIFIED};
    std::atomic<model::ThemeType>             mTheme{model::ThemeType::SystemDefault};
    std::atomic<LoadStage>                    mLoadStage{LoadStage::Uninit};

    // Set when loadSettings() is called before bindDatabase(); bindDatabase() then
    // auto-promotes the load once the DB id has been set.
    std::atomic<bool>                         mLoadPending{false};
};

} // namespace ucf::service
