#include "FeatureSettingsManager.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/DataWarehouseService/DatabaseConfig.h>
#include <ucf/Services/DataWarehouseService/IDataWarehouseService.h>
#include <ucf/Services/DataWarehouseSchema/DataWarehouseSchemas.h>
#include <ucf/Services/DataWarehouseService/DatabaseDataRecord.h>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>

#include "FeatureSettingsServiceLogger.h"

namespace ucf::service {

/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start FeatureSettingsManager Logic////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

FeatureSettingsManager::FeatureSettingsManager(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
{
    SERVICE_LOG_DEBUG("Create FeatureSettingsManager, address:" << this);
}

FeatureSettingsManager::~FeatureSettingsManager()
{
    SERVICE_LOG_DEBUG("Delete FeatureSettingsManager, address:" << this);
}

model::ScreenshotFeatureSettings FeatureSettingsManager::getScreenshotSettings() const
{
    std::lock_guard lock(mMutex);
    return mScreenshotSettings;
}

void FeatureSettingsManager::updateScreenshotSettings(const model::ScreenshotFeatureSettings& screenshotSettings)
{
    {
        std::lock_guard lock(mMutex);
        mScreenshotSettings = screenshotSettings;
    }
    saveScreenshotSettingsToDatabase();
}

void FeatureSettingsManager::databaseInitialized(const std::string& databaseId)
{
    SERVICE_LOG_DEBUG("Database initialized, databaseId:" << databaseId);
    loadScreenshotSettingsFromDatabase();
    loadRecordingSettingsFromDatabase();
}

void FeatureSettingsManager::loadScreenshotSettingsFromDatabase()
{
    auto coreFramework = mCoreFrameworkWPtr.lock();
    if (!coreFramework) return;

    auto dataWarehouseService = coreFramework->getService<ucf::service::IDataWarehouseService>().lock();
    if (!dataWarehouseService) return;

    auto clientInfoService = coreFramework->getService<ucf::service::IClientInfoService>().lock();
    if (!clientInfoService) return;

    const std::string databaseId = clientInfoService->getSharedDBConfig().getDBId();

    dataWarehouseService->fetchFromDatabase(
        databaseId,
        db::schema::ScreenshotSettingsTable::TableName,
        {
            db::schema::ScreenshotSettingsTable::SettingsIdentifierField,
            db::schema::ScreenshotSettingsTable::OutputDirectoryField,
            db::schema::ScreenshotSettingsTable::ImageFormatField,
            db::schema::ScreenshotSettingsTable::JpegQualityField,
            db::schema::ScreenshotSettingsTable::CaptureDelayField,
            db::schema::ScreenshotSettingsTable::AddTimestampField
        },
        {
            {db::schema::ScreenshotSettingsTable::SettingsIdentifierField, "default", ucf::service::model::DBOperatorType::Equal}
        },
        [this](const ucf::service::model::DatabaseDataRecords& results) {
            if (results.empty()) {
                SERVICE_LOG_DEBUG("No screenshot settings found in database, using defaults");
                return;
            }

            const auto& record = results.front();
            std::lock_guard lock(mMutex);

            auto outputDirectoryValue = record.getColumnData(db::schema::ScreenshotSettingsTable::OutputDirectoryField);
            mScreenshotSettings.outputDirectory = outputDirectoryValue.getStringValue();

            auto imageFormatValue = record.getColumnData(db::schema::ScreenshotSettingsTable::ImageFormatField);
            mScreenshotSettings.imageFormat = imageFormatValue.getStringValue();

            auto jpegQualityValue = record.getColumnData(db::schema::ScreenshotSettingsTable::JpegQualityField);
            mScreenshotSettings.jpegQuality = jpegQualityValue.getIntValue();

            auto captureDelayValue = record.getColumnData(db::schema::ScreenshotSettingsTable::CaptureDelayField);
            mScreenshotSettings.captureDelay = captureDelayValue.getIntValue();

            auto addTimestampValue = record.getColumnData(db::schema::ScreenshotSettingsTable::AddTimestampField);
            mScreenshotSettings.addTimestamp = (addTimestampValue.getIntValue() != 0);

            SERVICE_LOG_DEBUG("Loaded screenshot settings from database:"
                << " outputDirectory=" << mScreenshotSettings.outputDirectory
                << " imageFormat=" << mScreenshotSettings.imageFormat
                << " jpegQuality=" << mScreenshotSettings.jpegQuality
                << " captureDelay=" << mScreenshotSettings.captureDelay
                << " addTimestamp=" << mScreenshotSettings.addTimestamp);
        }
    );
}

void FeatureSettingsManager::saveScreenshotSettingsToDatabase()
{
    auto coreFramework = mCoreFrameworkWPtr.lock();
    if (!coreFramework) return;

    auto dataWarehouseService = coreFramework->getService<ucf::service::IDataWarehouseService>().lock();
    if (!dataWarehouseService) return;

    auto clientInfoService = coreFramework->getService<ucf::service::IClientInfoService>().lock();
    if (!clientInfoService) return;

    const std::string databaseId = clientInfoService->getSharedDBConfig().getDBId();

    model::ScreenshotFeatureSettings settingsCopy;
    {
        std::lock_guard lock(mMutex);
        settingsCopy = mScreenshotSettings;
    }

    ucf::service::model::ListOfDBValues databaseValues;
    databaseValues.emplace_back(ucf::service::model::DBDataValues{
        std::string("default"),
        settingsCopy.outputDirectory,
        settingsCopy.imageFormat,
        settingsCopy.jpegQuality,
        settingsCopy.captureDelay,
        settingsCopy.addTimestamp ? 1 : 0
    });

    dataWarehouseService->insertIntoDatabase(
        databaseId,
        db::schema::ScreenshotSettingsTable::TableName,
        {
            db::schema::ScreenshotSettingsTable::SettingsIdentifierField,
            db::schema::ScreenshotSettingsTable::OutputDirectoryField,
            db::schema::ScreenshotSettingsTable::ImageFormatField,
            db::schema::ScreenshotSettingsTable::JpegQualityField,
            db::schema::ScreenshotSettingsTable::CaptureDelayField,
            db::schema::ScreenshotSettingsTable::AddTimestampField
        },
        databaseValues
    );

    SERVICE_LOG_DEBUG("Saved screenshot settings to database:"
        << " outputDirectory=" << settingsCopy.outputDirectory
        << " imageFormat=" << settingsCopy.imageFormat
        << " jpegQuality=" << settingsCopy.jpegQuality
        << " captureDelay=" << settingsCopy.captureDelay
        << " addTimestamp=" << settingsCopy.addTimestamp);
}

/////////////////////////////////////////////////////////////////////////////////////
////////////////////Recording Settings///////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

model::RecordingFeatureSettings FeatureSettingsManager::getRecordingSettings() const
{
    std::lock_guard lock(mMutex);
    return mRecordingSettings;
}

void FeatureSettingsManager::updateRecordingSettings(const model::RecordingFeatureSettings& recordingSettings)
{
    {
        std::lock_guard lock(mMutex);
        mRecordingSettings = recordingSettings;
    }
    saveRecordingSettingsToDatabase();
}

void FeatureSettingsManager::loadRecordingSettingsFromDatabase()
{
    auto coreFramework = mCoreFrameworkWPtr.lock();
    if (!coreFramework) return;

    auto dataWarehouseService = coreFramework->getService<ucf::service::IDataWarehouseService>().lock();
    if (!dataWarehouseService) return;

    auto clientInfoService = coreFramework->getService<ucf::service::IClientInfoService>().lock();
    if (!clientInfoService) return;

    const std::string databaseId = clientInfoService->getSharedDBConfig().getDBId();

    dataWarehouseService->fetchFromDatabase(
        databaseId,
        db::schema::RecordingSettingsTable::TableName,
        {
            db::schema::RecordingSettingsTable::SettingsIdentifierField,
            db::schema::RecordingSettingsTable::OutputDirectoryField,
            db::schema::RecordingSettingsTable::VideoFormatField,
            db::schema::RecordingSettingsTable::FramesPerSecondField
        },
        {
            {db::schema::RecordingSettingsTable::SettingsIdentifierField, "default", ucf::service::model::DBOperatorType::Equal}
        },
        [this](const ucf::service::model::DatabaseDataRecords& results) {
            if (results.empty()) {
                SERVICE_LOG_DEBUG("No recording settings found in database, using defaults");
                return;
            }

            const auto& record = results.front();
            std::lock_guard lock(mMutex);

            auto outputDirectoryValue = record.getColumnData(db::schema::RecordingSettingsTable::OutputDirectoryField);
            mRecordingSettings.outputDirectory = outputDirectoryValue.getStringValue();

            auto videoFormatValue = record.getColumnData(db::schema::RecordingSettingsTable::VideoFormatField);
            mRecordingSettings.videoFormat = videoFormatValue.getStringValue();

            auto framesPerSecondValue = record.getColumnData(db::schema::RecordingSettingsTable::FramesPerSecondField);
            mRecordingSettings.framesPerSecond = framesPerSecondValue.getIntValue();

            SERVICE_LOG_DEBUG("Loaded recording settings from database:"
                << " outputDirectory=" << mRecordingSettings.outputDirectory
                << " videoFormat=" << mRecordingSettings.videoFormat
                << " framesPerSecond=" << mRecordingSettings.framesPerSecond);
        }
    );
}

void FeatureSettingsManager::saveRecordingSettingsToDatabase()
{
    auto coreFramework = mCoreFrameworkWPtr.lock();
    if (!coreFramework) return;

    auto dataWarehouseService = coreFramework->getService<ucf::service::IDataWarehouseService>().lock();
    if (!dataWarehouseService) return;

    auto clientInfoService = coreFramework->getService<ucf::service::IClientInfoService>().lock();
    if (!clientInfoService) return;

    const std::string databaseId = clientInfoService->getSharedDBConfig().getDBId();

    model::RecordingFeatureSettings settingsCopy;
    {
        std::lock_guard lock(mMutex);
        settingsCopy = mRecordingSettings;
    }

    ucf::service::model::ListOfDBValues databaseValues;
    databaseValues.emplace_back(ucf::service::model::DBDataValues{
        std::string("default"),
        settingsCopy.outputDirectory,
        settingsCopy.videoFormat,
        settingsCopy.framesPerSecond
    });

    dataWarehouseService->insertIntoDatabase(
        databaseId,
        db::schema::RecordingSettingsTable::TableName,
        {
            db::schema::RecordingSettingsTable::SettingsIdentifierField,
            db::schema::RecordingSettingsTable::OutputDirectoryField,
            db::schema::RecordingSettingsTable::VideoFormatField,
            db::schema::RecordingSettingsTable::FramesPerSecondField
        },
        databaseValues
    );

    SERVICE_LOG_DEBUG("Saved recording settings to database:"
        << " outputDirectory=" << settingsCopy.outputDirectory
        << " videoFormat=" << settingsCopy.videoFormat
        << " framesPerSecond=" << settingsCopy.framesPerSecond);
}

/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish FeatureSettingsManager Logic///////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

} // namespace ucf::service
