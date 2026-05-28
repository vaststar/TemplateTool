#pragma once

#include <ucf/Services/ServiceExportMacro/ServiceExport.h>
#include <ucf/Services/DataWarehouseService/DatabaseTableModel.h>
namespace db::schema{
struct SERVICE_EXPORT UserContactTable: public ucf::service::model::DBTableModel
{
    UserContactTable();
    static constexpr auto TableName            = "UserContact";
    static constexpr auto ContactIdField       = "CONTACT_ID";
    static constexpr auto ContactFullNameField = "CONTACT_FULL_NAME";
    static constexpr auto ContactStatusField   = "CONTACT_STATUS";
};

struct SERVICE_EXPORT GroupContactTable: public ucf::service::model::DBTableModel
{
    GroupContactTable();
    static constexpr auto TableName          = "GroupContact";
    static constexpr auto GroupIdField       = "GROUP_ID";
    static constexpr auto GroupNameField     = "GROUP_NAME";
    static constexpr auto ContactStatusField = "CONTACT_STATUS";
};

struct SERVICE_EXPORT ContactRelationTable: public ucf::service::model::DBTableModel
{
    ContactRelationTable();
    static constexpr auto TableName         = "ContactRelation";
    static constexpr auto ChildIdField      = "CHILD_ID";       // 主键，每个节点只挂在一个父下
    static constexpr auto ParentIdField     = "PARENT_ID";
    static constexpr auto RelationTypeField = "RELATION_TYPE";
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

struct SERVICE_EXPORT RecordingSettingsTable: public ucf::service::model::DBTableModel
{
    RecordingSettingsTable();
    static constexpr auto TableName = "RecordingSettings";
    static constexpr auto SettingsIdentifierField = "SETTINGS_IDENTIFIER";
    static constexpr auto OutputDirectoryField = "OUTPUT_DIRECTORY";
    static constexpr auto VideoFormatField = "VIDEO_FORMAT";
    static constexpr auto FramesPerSecondField = "FRAMES_PER_SECOND";
    static constexpr auto EnableMicrophoneField = "ENABLE_MICROPHONE";
    static constexpr auto EnableSystemAudioField = "ENABLE_SYSTEM_AUDIO";
    static constexpr auto MicDeviceIdField = "MIC_DEVICE_ID";
    static constexpr auto SystemAudioDeviceIdField = "SYSTEM_AUDIO_DEVICE_ID";
};

struct SERVICE_EXPORT CameraGroupTable: public ucf::service::model::DBTableModel
{
    CameraGroupTable();
    static constexpr auto TableName        = "CameraGroup";
    static constexpr auto NodeIdField      = "NODE_ID";
    static constexpr auto DisplayNameField = "DISPLAY_NAME";
    static constexpr auto NodeStatusField  = "NODE_STATUS";
};

struct SERVICE_EXPORT CameraTable: public ucf::service::model::DBTableModel
{
    CameraTable();
    static constexpr auto TableName             = "Camera";
    static constexpr auto NodeIdField           = "NODE_ID";
    static constexpr auto DisplayNameField      = "DISPLAY_NAME";
    static constexpr auto NodeStatusField       = "NODE_STATUS";
    static constexpr auto SourceTypeField       = "SOURCE_TYPE";        // 0=Local, 1=Network
    static constexpr auto LocalIndexField       = "LOCAL_INDEX";
    static constexpr auto NetworkUrlField       = "NETWORK_URL";
    static constexpr auto NetworkTransportField = "NETWORK_TRANSPORT";
    static constexpr auto OpenTimeoutMsField    = "OPEN_TIMEOUT_MS";
    static constexpr auto ReadTimeoutMsField    = "READ_TIMEOUT_MS";
};

struct SERVICE_EXPORT CameraDirectoryRelationTable: public ucf::service::model::DBTableModel
{
    CameraDirectoryRelationTable();
    static constexpr auto TableName         = "CameraDirectoryRelation";
    static constexpr auto ChildIdField      = "CHILD_ID";       // 主键，每个节点只挂在一个父下
    static constexpr auto ParentIdField     = "PARENT_ID";
    static constexpr auto RelationTypeField = "RELATION_TYPE";
};
}
