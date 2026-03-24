#pragma once

#include <ucf/Services/ServiceExportMacro/ServiceExport.h>
#include <ucf/Services/DataWarehouseService/DatabaseTableModel.h>
namespace db::schema{
struct SERVICE_EXPORT UserContactTable: public ucf::service::model::DBTableModel
{
    UserContactTable();
    static constexpr auto TableName = "UserContact";
    static constexpr auto ContactIdField = "CONTACT_ID";
    static constexpr auto ContactFullNameField = "CONTACT_FULL_NAME";
    static constexpr auto ContactEmailField = "CONTACT_EMAIL";
};

struct SERVICE_EXPORT GroupContactTable: public ucf::service::model::DBTableModel
{
    GroupContactTable();
    static constexpr auto TableName = "GroupContact";
    static constexpr auto GroupIdField = "GROUP_ID";
    static constexpr auto GroupNameField = "GROUP_NAME";
};

struct SERVICE_EXPORT SettingsTable: public ucf::service::model::DBTableModel
{
    SettingsTable();
    static constexpr auto TableName = "Settings";
    static constexpr auto KeyField = "Key";
    static constexpr auto ValField = "Value";
};

struct SERVICE_EXPORT ScreenshotSettingsTable: public ucf::service::model::DBTableModel
{
    ScreenshotSettingsTable();
    static constexpr auto TableName = "ScreenshotSettings";
    static constexpr auto SettingsIdentifierField = "SETTINGS_IDENTIFIER";
    static constexpr auto OutputDirectoryField = "OUTPUT_DIRECTORY";
    static constexpr auto ImageFormatField = "IMAGE_FORMAT";
    static constexpr auto JpegQualityField = "JPEG_QUALITY";
    static constexpr auto CaptureDelayField = "CAPTURE_DELAY";
    static constexpr auto AddTimestampField = "ADD_TIMESTAMP";
};
}
