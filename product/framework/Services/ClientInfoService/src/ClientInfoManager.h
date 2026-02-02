#pragma once

#include <memory>
#include <vector>
#include <atomic>
#include <ucf/Services/ClientInfoService/ClientInfoModel.h>


namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{

namespace model {
    class SqliteDBConfig;
}

class ClientInfoManager final
{
public:
    ClientInfoManager(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ~ClientInfoManager();
    ClientInfoManager(const ClientInfoManager&) = delete;
    ClientInfoManager(ClientInfoManager&&) = delete;
    ClientInfoManager& operator=(const ClientInfoManager&) = delete;
    ClientInfoManager& operator=(ClientInfoManager&&) = delete;
public:
    model::Version getApplicationVersion() const;

    model::ProductInfo getProductInfo() const;

    model::LanguageType getApplicationLanguage() const;
    void setApplicationLanguage(model::LanguageType languageType);
    std::vector<model::LanguageType> getSupportedLanguages() const;

    void setCurrentThemeType(model::ThemeType themeType);
    model::ThemeType getCurrentThemeType() const;
    std::vector<model::ThemeType> getSupportedThemeTypes() const;

    model::SqliteDBConfig getSharedDBConfig() const;
    void databaseInitialized(const std::string& dbId);

    std::string getDataStoragePath() const;
    std::string getLogStoragePath() const;
    std::string getCrashStoragePath() const;
private:
    const ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
    std::atomic<model::LanguageType> mLanguageType;
    std::atomic<model::ThemeType> mThemeType;
    std::atomic_bool mIsLanguageReadFromDB;
};
}