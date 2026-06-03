#pragma once

#include <memory>
#include <string>
#include <vector>

#include <ucf/Services/ClientInfoService/ClientInfoTypes.h>

#include "ClientInfoNotificationSink.h"

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{

namespace model {
    class SqliteDBConfig;
}

class ClientInfoModel;

// Aggregates client-side metadata (paths, version, product info) and delegates
// all mutable client settings (language / theme) to ClientInfoModel. Holds no
// reference to IDataWarehouseService; DB I/O lives entirely in ClientInfoModel
// via ClientInfoDBAccess.
class ClientInfoManager final
{
public:
    explicit ClientInfoManager(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ~ClientInfoManager();
    ClientInfoManager(const ClientInfoManager&) = delete;
    ClientInfoManager(ClientInfoManager&&) = delete;
    ClientInfoManager& operator=(const ClientInfoManager&) = delete;
    ClientInfoManager& operator=(ClientInfoManager&&) = delete;

public:
    // ===== Static metadata =====
    model::Version     getApplicationVersion() const;
    model::ProductInfo getProductInfo() const;

    // ===== Settings (delegated to ClientInfoModel) =====
    model::LanguageType              getApplicationLanguage() const;
    void                             setApplicationLanguage(model::LanguageType languageType);
    std::vector<model::LanguageType> getSupportedLanguages() const;

    void                          setCurrentThemeType(model::ThemeType themeType);
    model::ThemeType              getCurrentThemeType() const;
    std::vector<model::ThemeType> getSupportedThemeTypes() const;

    // ===== Shared DB lifecycle =====
    model::SqliteDBConfig getSharedDBConfig() const;
    void                  initializeAppClient();

    // Forwards to ClientInfoModel::bindDatabase after filtering events that do
    // not refer to the shared DB owned by this service. Only binds the DB id;
    // does NOT trigger any load — call loadSettings() explicitly for that.
    void databaseInitialized(const std::string& dbId);

    // Explicit load entry. Safe to call before databaseInitialized(): the request
    // is parked inside the Model and replayed when the DB id arrives.
    void loadSettings();

    // Late-subscriber short-circuit query, delegated to the Model.
    bool isReady() const;

    // Forwarded to the Model so the owning Service can implement the sink and
    // translate events into outward IClientInfoServiceCallback notifications.
    void setNotificationSink(std::weak_ptr<IClientInfoNotificationSink> sink);

    // ===== Path helpers =====
    std::string getDataStoragePath() const;
    std::string getLogStoragePath() const;
    std::string getCrashStoragePath() const;
    std::string getHangStoragePath() const;
    std::string getCacheStoragePath() const;
    std::string getTempStoragePath() const;
    std::string getExecutablePath() const;
    std::string getInstallDirectory() const;

private:
    const ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
    const std::unique_ptr<ClientInfoModel>   mClientInfoModel;
};
}
