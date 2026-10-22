#include "FeatureSettingsManager.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/services/DataWarehouseService/DatabaseConfig.h>
#include <ucf/services/DataWarehouseService/IDataWarehouseService.h>
#include <ucf/services/DataWarehouseSchema/DataWarehouseSchemas.h>
#include <ucf/services/DataWarehouseService/DatabaseDataRecord.h>
#include <ucf/services/ClientInfoService/IClientInfoService.h>

#include "FeatureSettingsServiceLogger.h"

#include <exception>

namespace ucf::service {

/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start FeatureSettingsManager Logic////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

FeatureSettingsManager::FeatureSettingsManager(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
{
    SERVICE_LOG_DEBUG("FeatureSettingsManager constructed, address: " << this);
}

FeatureSettingsManager::~FeatureSettingsManager()
{
    SERVICE_LOG_DEBUG("FeatureSettingsManager destroying, address: " << this);
}

model::ScreenshotFeatureSettings FeatureSettingsManager::getScreenshotSettings() const
{
    std::lock_guard lock(mMutex);
    return mScreenshotSettings;
}

bool FeatureSettingsManager::updateScreenshotSettings(const model::ScreenshotFeatureSettings& screenshotSettings)
{
    {
        std::lock_guard lock(mMutex);
        mScreenshotSettings = screenshotSettings;
    }
    return saveScreenshotSettingsToDatabase(screenshotSettings);
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

bool FeatureSettingsManager::saveScreenshotSettingsToDatabase(const model::ScreenshotFeatureSettings& screenshotSettings)
{
    try {
        auto coreFramework = mCoreFrameworkWPtr.lock();
        if (!coreFramework) {
            SERVICE_LOG_ERROR("Cannot persist screenshot settings: framework unavailable");
            return false;
        }

        auto dataWarehouseService = coreFramework->getService<ucf::service::IDataWarehouseService>().lock();
        if (!dataWarehouseService) {
            SERVICE_LOG_ERROR("Cannot persist screenshot settings: database service unavailable");
            return false;
        }

        auto clientInfoService = coreFramework->getService<ucf::service::IClientInfoService>().lock();
        if (!clientInfoService) {
            SERVICE_LOG_ERROR("Cannot persist screenshot settings: client info service unavailable");
            return false;
        }

        const std::string databaseId = clientInfoService->getSharedDBConfig().getDBId();

        ucf::service::model::ListOfDBValues databaseValues;
        databaseValues.emplace_back(ucf::service::model::DBDataValues{
            std::string("default"),
            screenshotSettings.outputDirectory,
            screenshotSettings.imageFormat,
            screenshotSettings.jpegQuality,
            screenshotSettings.captureDelay,
            screenshotSettings.addTimestamp ? 1 : 0
        });

        const bool persisted = dataWarehouseService->insertIntoDatabase(
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

        if (!persisted) {
            SERVICE_LOG_ERROR("Failed to persist screenshot settings to database: " << databaseId);
            return false;
        }

        SERVICE_LOG_DEBUG("Saved screenshot settings to database:"
            << " outputDirectory=" << screenshotSettings.outputDirectory
            << " imageFormat=" << screenshotSettings.imageFormat
            << " jpegQuality=" << screenshotSettings.jpegQuality
            << " captureDelay=" << screenshotSettings.captureDelay
            << " addTimestamp=" << screenshotSettings.addTimestamp);
        return true;
    } catch (const std::exception& error) {
        SERVICE_LOG_ERROR("Failed to persist screenshot settings: " << error.what());
    } catch (...) {
        SERVICE_LOG_ERROR("Failed to persist screenshot settings: unknown exception");
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////
////////////////////Recording Settings///////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

model::RecordingFeatureSettings FeatureSettingsManager::getRecordingSettings() const
{
    std::lock_guard lock(mMutex);
    return mRecordingSettings;
}

bool FeatureSettingsManager::updateRecordingSettings(const model::RecordingFeatureSettings& recordingSettings)
{
    {
        std::lock_guard lock(mMutex);
        mRecordingSettings = recordingSettings;
    }
    return saveRecordingSettingsToDatabase(recordingSettings);
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
            db::schema::RecordingSettingsTable::FramesPerSecondField,
            db::schema::RecordingSettingsTable::EnableMicrophoneField,
            db::schema::RecordingSettingsTable::EnableSystemAudioField,
            db::schema::RecordingSettingsTable::MicDeviceIdField,
            db::schema::RecordingSettingsTable::SystemAudioDeviceIdField
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

            auto enableMicValue = record.getColumnData(db::schema::RecordingSettingsTable::EnableMicrophoneField);
            mRecordingSettings.enableMicrophone = (enableMicValue.getIntValue() != 0);

            auto enableSysValue = record.getColumnData(db::schema::RecordingSettingsTable::EnableSystemAudioField);
            mRecordingSettings.enableSystemAudio = (enableSysValue.getIntValue() != 0);

            auto micDeviceIdValue = record.getColumnData(db::schema::RecordingSettingsTable::MicDeviceIdField);
            mRecordingSettings.micDeviceId = micDeviceIdValue.getStringValue();

            auto systemAudioDeviceIdValue = record.getColumnData(db::schema::RecordingSettingsTable::SystemAudioDeviceIdField);
            mRecordingSettings.systemAudioDeviceId = systemAudioDeviceIdValue.getStringValue();

            SERVICE_LOG_DEBUG("Loaded recording settings from database:"
                << " outputDirectory=" << mRecordingSettings.outputDirectory
                << " videoFormat=" << mRecordingSettings.videoFormat
                << " framesPerSecond=" << mRecordingSettings.framesPerSecond);
        }
    );
}

bool FeatureSettingsManager::saveRecordingSettingsToDatabase(const model::RecordingFeatureSettings& recordingSettings)
{
    try {
        auto coreFramework = mCoreFrameworkWPtr.lock();
        if (!coreFramework) {
            SERVICE_LOG_ERROR("Cannot persist recording settings: framework unavailable");
            return false;
        }

        auto dataWarehouseService = coreFramework->getService<ucf::service::IDataWarehouseService>().lock();
        if (!dataWarehouseService) {
            SERVICE_LOG_ERROR("Cannot persist recording settings: database service unavailable");
            return false;
        }

        auto clientInfoService = coreFramework->getService<ucf::service::IClientInfoService>().lock();
        if (!clientInfoService) {
            SERVICE_LOG_ERROR("Cannot persist recording settings: client info service unavailable");
            return false;
        }

        const std::string databaseId = clientInfoService->getSharedDBConfig().getDBId();

        ucf::service::model::ListOfDBValues databaseValues;
        databaseValues.emplace_back(ucf::service::model::DBDataValues{
            std::string("default"),
            recordingSettings.outputDirectory,
            recordingSettings.videoFormat,
            recordingSettings.framesPerSecond,
            static_cast<int>(recordingSettings.enableMicrophone),
            static_cast<int>(recordingSettings.enableSystemAudio),
            recordingSettings.micDeviceId,
            recordingSettings.systemAudioDeviceId
        });

        const bool persisted = dataWarehouseService->insertIntoDatabase(
            databaseId,
            db::schema::RecordingSettingsTable::TableName,
            {
                db::schema::RecordingSettingsTable::SettingsIdentifierField,
                db::schema::RecordingSettingsTable::OutputDirectoryField,
                db::schema::RecordingSettingsTable::VideoFormatField,
                db::schema::RecordingSettingsTable::FramesPerSecondField,
                db::schema::RecordingSettingsTable::EnableMicrophoneField,
                db::schema::RecordingSettingsTable::EnableSystemAudioField,
                db::schema::RecordingSettingsTable::MicDeviceIdField,
                db::schema::RecordingSettingsTable::SystemAudioDeviceIdField
            },
            databaseValues
        );

        if (!persisted) {
            SERVICE_LOG_ERROR("Failed to persist recording settings to database: " << databaseId);
            return false;
        }

        SERVICE_LOG_DEBUG("Saved recording settings to database:"
            << " outputDirectory=" << recordingSettings.outputDirectory
            << " videoFormat=" << recordingSettings.videoFormat
            << " framesPerSecond=" << recordingSettings.framesPerSecond
            << " enableMicrophone=" << recordingSettings.enableMicrophone
            << " enableSystemAudio=" << recordingSettings.enableSystemAudio);
        return true;
    } catch (const std::exception& error) {
        SERVICE_LOG_ERROR("Failed to persist recording settings: " << error.what());
    } catch (...) {
        SERVICE_LOG_ERROR("Failed to persist recording settings: unknown exception");
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish FeatureSettingsManager Logic///////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

} // namespace ucf::service
