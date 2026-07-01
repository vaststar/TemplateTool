#include "ClientInfoService.h"

#include <locale>
#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Utilities/TimeUtils/TimeUtils.h>
#include <ucf/Utilities/OSUtils/OSUtils.h>
#include <ucf/Utilities/StringUtils/StringUtils.h>

#include <ucf/Services/DataWarehouseService/DatabaseConfig.h>
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
    ClientInfoManager& getClientInfoManager();
    const ClientInfoManager& getClientInfoManager() const;
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

ClientInfoManager& ClientInfoService::DataPrivate::getClientInfoManager()
{
    return *mClientInfoManager;
}

const ClientInfoManager& ClientInfoService::DataPrivate::getClientInfoManager() const
{
    return *mClientInfoManager;
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
    // Wire reverse notification via internal sink: the model invokes our sink
    // methods after a state change and we forward them through NotificationHelper.
    // Sink is stored as weak_ptr inside the model so async DB callbacks arriving
    // after teardown are safe.
    mDataPrivate->getClientInfoManager().setNotificationSink(
        std::weak_ptr<IClientInfoNotificationSink>(
            std::static_pointer_cast<IClientInfoNotificationSink>(shared_from_this())));

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

void ClientInfoService::deinitService()
{
    SERVICE_LOG_DEBUG("ClientInfoService::deinitService()");
}

std::string ClientInfoService::getServiceName() const
{
    return "ClientInfoService";
}

std::vector<ServiceDependency> ClientInfoService::dependencies() const
{
    return { { std::type_index(typeid(IDataWarehouseService)), DependencyKind::Required } };
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
    return mDataPrivate->getClientInfoManager().getApplicationVersion();
}

model::ProductInfo ClientInfoService::getProductInfo() const
{
    return mDataPrivate->getClientInfoManager().getProductInfo();
}

void ClientInfoService::printClientInfo() const
{
    SERVICE_LOG_DEBUG("==================================");
    SERVICE_LOG_DEBUG("===========ClientInfo=============");
    SERVICE_LOG_DEBUG("app data storage path: " << getAppDataStoragePath());
    SERVICE_LOG_DEBUG("app log storage path: " << getAppLogStoragePath());
    SERVICE_LOG_DEBUG("app crash storage path: " << getAppCrashStoragePath());
    SERVICE_LOG_DEBUG("app hang storage path: " << getAppHangStoragePath());
    SERVICE_LOG_DEBUG("app cache storage path: " << getAppCacheStoragePath());
    SERVICE_LOG_DEBUG("app temp storage path: " << getAppTempStoragePath());
    SERVICE_LOG_DEBUG("executable path: " << getExecutablePath());
    SERVICE_LOG_DEBUG("install directory: " << getInstallDirectory());
    SERVICE_LOG_DEBUG("product name: " << getProductInfo().productName);
    SERVICE_LOG_DEBUG("copyright: " << getProductInfo().copyright);
    SERVICE_LOG_DEBUG("company name: " << getProductInfo().companyName);
    SERVICE_LOG_DEBUG("product description: " << getProductInfo().productDescription);
    SERVICE_LOG_DEBUG("client version: " << getApplicationVersion().toString());
    SERVICE_LOG_DEBUG("client compiler: " << ucf::utilities::OSUtils::getCompilerInfo());
    SERVICE_LOG_DEBUG("os platform: " << ucf::utilities::OSUtils::getOSTypeName());
    SERVICE_LOG_DEBUG("os version: " << ucf::utilities::OSUtils::getOSVersion());
    SERVICE_LOG_DEBUG("os cpu core: " << std::to_string(ucf::utilities::OSUtils::getCPUCoreCount()));
    SERVICE_LOG_DEBUG("os cpu arch: " << ucf::utilities::OSUtils::getCPUArch());
    SERVICE_LOG_DEBUG("os cpu info: " << ucf::utilities::OSUtils::getCPUInfo());
    SERVICE_LOG_DEBUG("os local language: " << ucf::utilities::OSUtils::getSystemLanguage());
    SERVICE_LOG_DEBUG("os timezone: " << ucf::utilities::TimeUtils::getLocalTimeZoneName());
    SERVICE_LOG_DEBUG("os total memory: " << ucf::utilities::OSUtils::getMemoryInfo().totalMemoryBytes / (1024 * 1024) << " MB");
    SERVICE_LOG_DEBUG("os available memory: " << ucf::utilities::OSUtils::getMemoryInfo().availableMemoryBytes / (1024 * 1024) << " MB");
    SERVICE_LOG_DEBUG("os gpu info: " << ucf::utilities::OSUtils::getGPUInfo());
    SERVICE_LOG_DEBUG("===========ClientInfo=============");
    printBuildInfo();
    SERVICE_LOG_DEBUG("==================================");
}

void ClientInfoService::printBuildInfo() const
{
    SERVICE_LOG_DEBUG("===========BuildInfo=============");
    #ifdef NDEBUG
        SERVICE_LOG_DEBUG("build type: Release");
    #else
        SERVICE_LOG_DEBUG("build type: Debug");
    #endif
    SERVICE_LOG_DEBUG("build timestamp: " << __DATE__ << " " << __TIME__);
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
    return mDataPrivate->getClientInfoManager().getApplicationLanguage();
}

void ClientInfoService::setApplicationLanguage(model::LanguageType languageType)
{
    mDataPrivate->getClientInfoManager().setApplicationLanguage(languageType);
}

std::vector<model::LanguageType> ClientInfoService::getSupportedLanguages() const
{
    return mDataPrivate->getClientInfoManager().getSupportedLanguages();
}

void ClientInfoService::setCurrentThemeType(model::ThemeType themeType)
{
    // Model is the single source of truth; it dedupes, persists, and fires the
    // sink (which we translate into onClientThemeChanged below).
    mDataPrivate->getClientInfoManager().setCurrentThemeType(themeType);
}

model::ThemeType ClientInfoService::getCurrentThemeType() const
{
    return mDataPrivate->getClientInfoManager().getCurrentThemeType();
}

std::vector<model::ThemeType> ClientInfoService::getSupportedThemeTypes() const
{
    return mDataPrivate->getClientInfoManager().getSupportedThemeTypes();
}

model::SqliteDBConfig ClientInfoService::getSharedDBConfig() const
{
    return mDataPrivate->getClientInfoManager().getSharedDBConfig();
}

std::string ClientInfoService::getAppDataStoragePath() const
{
    return mDataPrivate->getClientInfoManager().getDataStoragePath();
}

std::string ClientInfoService::getAppLogStoragePath() const
{
    return mDataPrivate->getClientInfoManager().getLogStoragePath();
}

std::string ClientInfoService::getAppCrashStoragePath() const
{
    return mDataPrivate->getClientInfoManager().getCrashStoragePath();
}

std::string ClientInfoService::getAppHangStoragePath() const
{
    return mDataPrivate->getClientInfoManager().getHangStoragePath();
}

std::string ClientInfoService::getAppCacheStoragePath() const
{
    return mDataPrivate->getClientInfoManager().getCacheStoragePath();
}

std::string ClientInfoService::getAppTempStoragePath() const
{
    return mDataPrivate->getClientInfoManager().getTempStoragePath();
}

std::string ClientInfoService::getExecutablePath() const
{
    return mDataPrivate->getClientInfoManager().getExecutablePath();
}

std::string ClientInfoService::getInstallDirectory() const
{
    return mDataPrivate->getClientInfoManager().getInstallDirectory();
}

void ClientInfoService::initializeAppClient()
{
    mDataPrivate->getClientInfoManager().initializeAppClient();
}

void ClientInfoService::OnDatabaseInitialized(const std::string& dbId)
{
    mDataPrivate->getClientInfoManager().databaseInitialized(dbId);
}

bool ClientInfoService::isClientInfoReady() const
{
    return mDataPrivate->getClientInfoManager().isReady();
}

// ===== IClientInfoNotificationSink =====

void ClientInfoService::onClientInfoReady()
{
    SERVICE_LOG_DEBUG("fire onClientInfoReady");
    fireNotification(&IClientInfoServiceCallback::onClientInfoReady);
}

void ClientInfoService::onClientInfoLoadFailed(ClientInfoLoadError error)
{
    SERVICE_LOG_ERROR("fire onClientInfoLoadFailed, error:" << static_cast<int>(error));
    fireNotification(&IClientInfoServiceCallback::onClientInfoLoadFailed, error);
}

void ClientInfoService::onClientLanguageChanged(model::LanguageType languageType)
{
    SERVICE_LOG_DEBUG("fire onClientLanguageChanged, language:" << static_cast<int>(languageType));
    fireNotification(&IClientInfoServiceCallback::onClientLanguageChanged, languageType);
}

void ClientInfoService::onClientThemeChanged(model::ThemeType themeType)
{
    SERVICE_LOG_DEBUG("fire onClientThemeChanged, theme:" << static_cast<int>(themeType));
    fireNotification(&IClientInfoServiceCallback::onClientThemeChanged, themeType);
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish ClientInfoService Logic///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}
