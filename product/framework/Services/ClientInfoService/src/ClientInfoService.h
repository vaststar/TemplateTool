#pragma once

#include <memory>

#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>

#include <ucf/CoreFramework/CoreFrameworkCallbackDefault.h>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>
#include <ucf/Services/DataWarehouseService/IDataWarehouseServiceCallback.h>


namespace ucf::service{
class SERVICE_EXPORT ClientInfoService final: public virtual IClientInfoService, 
                               public virtual ucf::utilities::NotificationHelper<IClientInfoServiceCallback>,
                               public ucf::framework::CoreFrameworkCallbackDefault,
                               public ucf::service::IDataWarehouseServiceCallback,
                               public std::enable_shared_from_this<ClientInfoService>
{
public:
    explicit ClientInfoService(ucf::framework::ICoreFrameworkWPtr coreFramework);
    virtual ~ClientInfoService();
    ClientInfoService(const ClientInfoService&) = delete;
    ClientInfoService(ClientInfoService&&) = delete;
    ClientInfoService& operator=(const ClientInfoService&) = delete;
    ClientInfoService& operator=(ClientInfoService&&) = delete;

public:
    //IService
    virtual std::string getServiceName() const override;

    //ICoreFrameworkCallback
    virtual void onServiceInitialized() override;
    virtual void onCoreFrameworkExit() override;

    //IDataWarehouseServiceCallback
    virtual void OnDatabaseInitialized(const std::string& dbId) override;

    //IClientInfoService
    virtual model::Version getApplicationVersion() const override;

    virtual model::ProductInfo getProductInfo() const override;

    virtual model::LanguageType getApplicationLanguage() const override;
    virtual void setApplicationLanguage(model::LanguageType languageType) override;
    virtual std::vector<model::LanguageType> getSupportedLanguages() const override;

    virtual void setCurrentThemeType(model::ThemeType themeType) override;
    virtual model::ThemeType getCurrentThemeType() const override;
    virtual std::vector<model::ThemeType> getSupportedThemeTypes() const override;

    virtual model::SqliteDBConfig getSharedDBConfig() const override;
    
    virtual std::string getAppDataStoragePath() const override;
    virtual std::string getAppLogStoragePath() const override;
protected:
    //IService
    virtual void initService() override;
private:
    void printClientInfo() const;
    void printBuildInfo() const;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}