#include "ClientInfoService.h"

#include <locale>
#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Utilities/TimeUtils/TimeUtils.h>
#include <ucf/Utilities/OSUtils/OSUtils.h>
#include <ucf/Utilities/StringUtils/StringUtils.h>

#include <ucf/Services/DataWarehouseService/DataBaseConfig.h>
#include <ucf/Services/DataWarehouseService/IDataWarehouseService.h>

#include <magic_enum/magic_enum.hpp>

#include "ClientInfoManager.h"
#include "ClientInfoServiceLogger.h"

namespace ucf::service{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class ClientInfoService::DataPrivate{
public:
    explicit DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ucf::framework::ICoreFrameworkWPtr getCoreFramework() const;
    ClientInfoManager* getClientInfoManager() const;
private:
    ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
    std::unique_ptr<ClientInfoManager> mClientInfoManager;
};

ClientInfoService::DataPrivate::DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
    , mClientInfoManager(std::make_unique<ClientInfoManager>(coreFramework))
{

}

ucf::framework::ICoreFrameworkWPtr ClientInfoService::DataPrivate::getCoreFramework() const
{
    return mCoreFrameworkWPtr;
}

ClientInfoManager* ClientInfoService::DataPrivate::getClientInfoManager() const
{
    return mClientInfoManager.get();
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start ClientInfoService Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<IClientInfoService> IClientInfoService::createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework)
{
    return std::make_shared<ClientInfoService>(coreFramework);
}

ClientInfoService::ClientInfoService(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mDataPrivate(std::make_unique<DataPrivate>(coreFramework))
{
    SERVICE_LOG_DEBUG("Create ClientInfoService, address:" << this);
}

ClientInfoService::~ClientInfoService()
{
    SERVICE_LOG_DEBUG("Delete ClientInfoService, address:" << this);
}

void ClientInfoService::initService()
{
    if (auto coreFramework = mDataPrivate->getCoreFramework().lock())
    {
        coreFramework->registerCallback(shared_from_this());
        if (auto dataWarehouse = coreFramework->getService<ucf::service::IDataWarehouseService>().lock())
        {
            dataWarehouse->registerCallback(shared_from_this());
        }
    }
    printClientInfo();
}

std::string ClientInfoService::getServiceName() const
{
    return "ClientInfoService";
}

void ClientInfoService::onServiceInitialized()
{
    SERVICE_LOG_DEBUG("");
}

void ClientInfoService::onCoreFrameworkExit()
{
    SERVICE_LOG_DEBUG("");
}

model::Version ClientInfoService::getApplicationVersion() const
{
    return mDataPrivate->getClientInfoManager()->getApplicationVersion();
}

void ClientInfoService::printClientInfo() const
{
    SERVICE_LOG_DEBUG("==================================");
    SERVICE_LOG_DEBUG("===========ClientInfo=============");
    SERVICE_LOG_DEBUG("client version: " << getApplicationVersion().toString());
    SERVICE_LOG_DEBUG("client compiler: " << ucf::utilities::OSUtils::getCompilerInfo());
    SERVICE_LOG_DEBUG("os platform: " << ucf::utilities::OSUtils::getOSTypeName());
    SERVICE_LOG_DEBUG("os version: " << ucf::utilities::OSUtils::getOSVersion());
    SERVICE_LOG_DEBUG("os cpu core: " << std::to_string(ucf::utilities::OSUtils::getCPUCoreCount()));
    SERVICE_LOG_DEBUG("os cpu info: " << ucf::utilities::OSUtils::getCPUInfo());
    SERVICE_LOG_DEBUG("os local language: " << ucf::utilities::OSUtils::getSystemLanguage());
    SERVICE_LOG_DEBUG("os timezone: " << ucf::utilities::TimeUtils::getLocalTimeZone());
    SERVICE_LOG_DEBUG("os total memory: " << ucf::utilities::OSUtils::getMemoryInfo().totalMemoryBytes / (1024 * 1024) << " MB");
    SERVICE_LOG_DEBUG("os available memory: " << ucf::utilities::OSUtils::getMemoryInfo().availableMemoryBytes / (1024 * 1024) << " MB");
    SERVICE_LOG_DEBUG("os gpu info: " << ucf::utilities::OSUtils::getGPUInfo());
    
    // SERVICE_LOG_DEBUG("client language: " << std::to_string(static_cast<std::underlying_type<decltype(getApplicationLanguage())>::type>(getApplicationLanguage())));
    // SERVICE_LOG_DEBUG("client language: " << magic_enum::enum_name(getApplicationLanguage()));
    //SERVICE_LOG_DEBUG("client language: " << ucf::utilities::StringUtils::enumToString(getApplicationLanguage()));
    SERVICE_LOG_DEBUG("===========ClientInfo=============");
    printBuildInfo();
    SERVICE_LOG_DEBUG("==================================");
}

void ClientInfoService::printBuildInfo() const
{
    SERVICE_LOG_DEBUG("===========BuildInfo=============");
    #ifdef CMAKE_VERSION_STR
        SERVICE_LOG_DEBUG("CMake version: " << CMAKE_VERSION_STR);
    #else
        SERVICE_LOG_DEBUG("CMake version macro not defined");
    #endif
    #ifdef CMAKE_COMPILER_ID_STR
        SERVICE_LOG_DEBUG("C++ Compiler ID: " << CMAKE_COMPILER_ID_STR);
    #else
        SERVICE_LOG_DEBUG("C++ Compiler ID macro not defined");
    #endif
    #ifdef CMAKE_COMPILER_VERSION_STR
        SERVICE_LOG_DEBUG("C++ Compiler version: " << CMAKE_COMPILER_VERSION_STR);
    #else
        SERVICE_LOG_DEBUG("C++ Compiler version macro not defined");
    #endif
    #ifdef CMAKE_COMPILER_PATH_STR
        SERVICE_LOG_DEBUG("C++ Compiler path: " << CMAKE_COMPILER_PATH_STR);
    #else
        SERVICE_LOG_DEBUG("C++ Compiler path macro not defined");
    #endif
    SERVICE_LOG_DEBUG("===========BuildInfo=============");
}

model::LanguageType ClientInfoService::getApplicationLanguage() const
{
    return mDataPrivate->getClientInfoManager()->getApplicationLanguage();
}

void ClientInfoService::setApplicationLanguage(model::LanguageType languageType)
{
    mDataPrivate->getClientInfoManager()->setApplicationLanguage(languageType);
}

std::vector<model::LanguageType> ClientInfoService::getSupportedLanguages() const
{
    return mDataPrivate->getClientInfoManager()->getSupportedLanguages();
}

void ClientInfoService::setCurrentThemeType(model::ThemeType themeType)
{
    if (getCurrentThemeType() != themeType)
    {
        mDataPrivate->getClientInfoManager()->setCurrentThemeType(themeType);
        fireNotification(&IClientInfoServiceCallback::onClientThemeChanged, themeType);
    }
}

model::ThemeType ClientInfoService::getCurrentThemeType() const
{
    return mDataPrivate->getClientInfoManager()->getCurrentThemeType();
}

std::vector<model::ThemeType> ClientInfoService::getSupportedThemeTypes() const
{
    return mDataPrivate->getClientInfoManager()->getSupportedThemeTypes();
}

model::SqliteDBConfig ClientInfoService::getSharedDBConfig() const
{
    return mDataPrivate->getClientInfoManager()->getSharedDBConfig();
}

void ClientInfoService::OnDatabaseInitialized(const std::string& dbId)
{
    mDataPrivate->getClientInfoManager()->databaseInitialized(dbId);
    fireNotification(&IClientInfoServiceCallback::onClientInfoReady);
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish ClientInfoService Logic///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}