#include "ClientInfoManager.h"

#include <filesystem>
#include <utility>

#include <appVersion.h>
#include <ucf/CoreFramework/ICoreFramework.h>

#include <ucf/Services/DataWarehouseSchema/DataWarehouseSchemas.h>
#include <ucf/Services/DataWarehouseService/DatabaseConfig.h>
#include <ucf/Services/DataWarehouseService/IDataWarehouseService.h>

#include <ucf/Utilities/FilePathUtils/FilePathUtils.h>
#include <ucf/Utilities/SystemUtils/SystemUtils.h>
#include <ucf/Utilities/UUIDUtils/UUIDUtils.h>

#include "ClientInfoModel.h"
#include "ClientInfoServiceLogger.h"

#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#else
#include <unistd.h>
#endif

namespace ucf::service{
static constexpr const char* APP_INTERNAL_NAME = "TemplateToolApp";
static constexpr const char* APP_INTERNAL_NAME_DEBUG = "TemplateToolAppDebug";
static constexpr const char* APP_DATA_FOLDER_NAME = "app_data";
static constexpr const char* APP_LOG_FOLDER_NAME = "app_log";
static constexpr const char* APP_CRASH_FOLDER_NAME = "app_crash";
static constexpr const char* APP_HANG_FOLDER_NAME = "app_hang";
static constexpr const char* APP_CACHE_FOLDER_NAME = "app_cache";

ClientInfoManager::ClientInfoManager(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
    , mClientInfoModel(std::make_unique<ClientInfoModel>(coreFramework))
{
    SERVICE_LOG_DEBUG("Create ClientInfoManager, address:" << this);
    if (!ucf::utilities::FilePathUtils::EnsureDirectoryExists(getDataStoragePath()))
    {
        SERVICE_LOG_DEBUG("Create data storage directory failed, path:" << getDataStoragePath());
    }
    if (!ucf::utilities::FilePathUtils::EnsureDirectoryExists(getLogStoragePath()))
    {
        SERVICE_LOG_DEBUG("Create log storage directory failed, path:" << getLogStoragePath());
    }
}

ClientInfoManager::~ClientInfoManager() = default;

// ===== Static metadata =====

model::Version ClientInfoManager::getApplicationVersion() const
{
    return model::Version{AppVersion::VERSION_MAJOR, AppVersion::VERSION_MINOR, AppVersion::VERSION_PATCH, AppVersion::VERSION_BUILD};
}

model::ProductInfo ClientInfoManager::getProductInfo() const
{
    return model::ProductInfo{
        AppInfo::COMPANY_NAME,
        AppInfo::COPYRIGHT,
        AppInfo::PRODUCT_NAME,
        AppInfo::PRODUCT_DESCRIPTION
    };
}

// ===== Settings (delegated) =====

model::LanguageType ClientInfoManager::getApplicationLanguage() const
{
    return mClientInfoModel->getLanguage();
}

void ClientInfoManager::setApplicationLanguage(model::LanguageType languageType)
{
    mClientInfoModel->setLanguage(languageType);
}

std::vector<model::LanguageType> ClientInfoManager::getSupportedLanguages() const
{
    return {model::LanguageType::ENGLISH, model::LanguageType::CHINESE_SIMPLIFIED, model::LanguageType::CHINESE_TRADITIONAL, model::LanguageType::FRENCH, model::LanguageType::GERMAN,
            model::LanguageType::ITALIAN, model::LanguageType::SPANISH, model::LanguageType::PORTUGUESE, model::LanguageType::JAPANESE, model::LanguageType::KOREAN, model::LanguageType::RUSSIAN};
}

void ClientInfoManager::setCurrentThemeType(model::ThemeType themeType)
{
    mClientInfoModel->setTheme(themeType);
}

model::ThemeType ClientInfoManager::getCurrentThemeType() const
{
    return mClientInfoModel->getTheme();
}

std::vector<model::ThemeType> ClientInfoManager::getSupportedThemeTypes() const
{
    return {model::ThemeType::SystemDefault, model::ThemeType::Dark, model::ThemeType::Light};
}

// ===== Shared DB lifecycle =====

model::SqliteDBConfig ClientInfoManager::getSharedDBConfig() const
{
    return model::SqliteDBConfig("test", ucf::utilities::FilePathUtils::joinPaths(std::filesystem::path(getDataStoragePath()), "shared_database.db").string(), "");
}

void ClientInfoManager::initializeAppClient()
{
    auto coreFramework = mCoreFrameworkWPtr.lock();
    if (!coreFramework)
    {
        return;
    }
    auto dataWarehouseService = coreFramework->getService<ucf::service::IDataWarehouseService>().lock();
    if (!dataWarehouseService)
    {
        SERVICE_LOG_ERROR("initializeAppClient: DataWarehouseService unavailable");
        return;
    }

    auto dbConfig = getSharedDBConfig();
    SERVICE_LOG_DEBUG("initializeAppClient with dbId:" << dbConfig.getDBId() << ", dbFilePath:" << dbConfig.getDBFilePath());
    std::vector<ucf::service::model::DBTableModel> tables{
        db::schema::UserContactTable{},
        db::schema::PersonContactTable{},
        db::schema::GroupContactTable{},
        db::schema::DepartmentGroupTable{},
        db::schema::TeamGroupTable{},
        db::schema::FolderGroupTable{},
        db::schema::ContactRelationTable{},
        db::schema::SettingsTable{},
        db::schema::ScreenshotSettingsTable{},
        db::schema::RecordingSettingsTable{},
        db::schema::CameraGroupTable{},
        db::schema::CameraTable{},
        db::schema::CameraDirectoryRelationTable{}
    };
    dataWarehouseService->initializeDB(
        std::make_shared<ucf::service::model::SqliteDBConfig>(dbConfig.getDBId(), dbConfig.getDBFilePath()),
        tables);
}

void ClientInfoManager::databaseInitialized(const std::string& dbId)
{
    if (const auto myDbId = getSharedDBConfig().getDBId(); dbId != myDbId)
    {
        SERVICE_LOG_DEBUG("databaseInitialized ignored, other db, dbId:" << dbId);
        return;
    }
    mClientInfoModel->bindDatabase(dbId);

    SERVICE_LOG_DEBUG("Loading client settings after database initialized, dbId:" << dbId);
    loadSettings();
}

void ClientInfoManager::loadSettings()
{
    mClientInfoModel->loadSettings();
}

bool ClientInfoManager::isReady() const
{
    return mClientInfoModel->isReady();
}

void ClientInfoManager::setNotificationSink(std::weak_ptr<IClientInfoNotificationSink> sink)
{
    SinkNotifier::setNotificationSink(sink);
    mClientInfoModel->setNotificationSink(std::move(sink));
}

// ===== Path helpers =====

std::string ClientInfoManager::getDataStoragePath() const
{
#if defined(_DEBUG) || !defined(NDEBUG)
    return ucf::utilities::FilePathUtils::joinPaths(
        ucf::utilities::SystemUtils::getBaseStorageDir(),
        APP_INTERNAL_NAME_DEBUG,
        APP_DATA_FOLDER_NAME
    ).string();
#else
    return ucf::utilities::FilePathUtils::joinPaths(
        ucf::utilities::SystemUtils::getBaseStorageDir(),
        APP_INTERNAL_NAME,
        APP_DATA_FOLDER_NAME
    ).string();
#endif
}

std::string ClientInfoManager::getLogStoragePath() const
{
#if defined(_DEBUG) || !defined(NDEBUG)
    return ucf::utilities::FilePathUtils::joinPaths(
        ucf::utilities::SystemUtils::getBaseStorageDir(),
        APP_INTERNAL_NAME_DEBUG,
        APP_LOG_FOLDER_NAME
    ).string();
#else
    return ucf::utilities::FilePathUtils::joinPaths(
        ucf::utilities::SystemUtils::getBaseStorageDir(),
        APP_INTERNAL_NAME,
        APP_LOG_FOLDER_NAME
    ).string();
#endif
}

std::string ClientInfoManager::getCrashStoragePath() const
{
#if defined(_DEBUG) || !defined(NDEBUG)
    return ucf::utilities::FilePathUtils::joinPaths(
        ucf::utilities::SystemUtils::getBaseStorageDir(),
        APP_INTERNAL_NAME_DEBUG,
        APP_CRASH_FOLDER_NAME
    ).string();
#else
    return ucf::utilities::FilePathUtils::joinPaths(
        ucf::utilities::SystemUtils::getBaseStorageDir(),
        APP_INTERNAL_NAME,
        APP_CRASH_FOLDER_NAME
    ).string();
#endif
}

std::string ClientInfoManager::getHangStoragePath() const
{
#if defined(_DEBUG) || !defined(NDEBUG)
    return ucf::utilities::FilePathUtils::joinPaths(
        ucf::utilities::SystemUtils::getBaseStorageDir(),
        APP_INTERNAL_NAME_DEBUG,
        APP_HANG_FOLDER_NAME
    ).string();
#else
    return ucf::utilities::FilePathUtils::joinPaths(
        ucf::utilities::SystemUtils::getBaseStorageDir(),
        APP_INTERNAL_NAME,
        APP_HANG_FOLDER_NAME
    ).string();
#endif
}

std::string ClientInfoManager::getCacheStoragePath() const
{
#if defined(_DEBUG) || !defined(NDEBUG)
    return ucf::utilities::FilePathUtils::joinPaths(
        ucf::utilities::SystemUtils::getBaseCacheDir(),
        APP_INTERNAL_NAME_DEBUG,
        APP_CACHE_FOLDER_NAME
    ).string();
#else
    return ucf::utilities::FilePathUtils::joinPaths(
        ucf::utilities::SystemUtils::getBaseCacheDir(),
        APP_INTERNAL_NAME,
        APP_CACHE_FOLDER_NAME
    ).string();
#endif
}

std::string ClientInfoManager::getTempStoragePath() const
{
#if defined(_DEBUG) || !defined(NDEBUG)
    return (std::filesystem::temp_directory_path() / APP_INTERNAL_NAME_DEBUG).string();
#else
    return (std::filesystem::temp_directory_path() / APP_INTERNAL_NAME).string();
#endif
}

std::string ClientInfoManager::getExecutablePath() const
{
#if defined(__APPLE__)
    char buf[4096];
    uint32_t size = sizeof(buf);
    if (_NSGetExecutablePath(buf, &size) == 0) {
        return std::filesystem::canonical(buf).string();
    }
    return {};
#elif defined(__linux__)
    char buf[4096];
    auto len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len > 0) {
        buf[len] = '\0';
        return std::string(buf);
    }
    return {};
#elif defined(_WIN32)
    wchar_t buf[MAX_PATH];
    GetModuleFileNameW(nullptr, buf, MAX_PATH);
    return std::filesystem::path(buf).string();
#else
    return {};
#endif
}

std::string ClientInfoManager::getInstallDirectory() const
{
    auto execPath = std::filesystem::path(getExecutablePath());
    if (execPath.empty()) {
        return {};
    }
#if defined(__APPLE__)
    // macOS: execPath = /path/to/mainEntry.app/Contents/MacOS/mainEntry
    // Install directory is the .app bundle itself
    return execPath.parent_path().parent_path().parent_path().string();
#else
    // Windows & Linux: execPath = /install-root/bin/mainEntry(.exe)
    // Install directory is parent of bin/
    return execPath.parent_path().parent_path().string();
#endif
}

}
