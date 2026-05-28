#include <ucf/Services/DataWarehouseSchema/DataWarehouseSchemas.h>

namespace db::schema{
UserContactTable::UserContactTable()
    : ucf::service::model::DBTableModel(TableName,{
        {ContactIdField,       "TEXT UNIQUE NOT NULL"},
        {ContactFullNameField, "TEXT NOT NULL"},
        {ContactStatusField,   "INTEGER"}
    })
{
}

GroupContactTable::GroupContactTable()
    : ucf::service::model::DBTableModel(TableName,{
        {GroupIdField,         "TEXT UNIQUE NOT NULL"},
        {GroupNameField,       "TEXT NOT NULL"},
        {ContactStatusField,   "INTEGER"}
    })
{
}

ContactRelationTable::ContactRelationTable()
    : ucf::service::model::DBTableModel(TableName,{
        {ChildIdField,      "TEXT UNIQUE NOT NULL"},
        {ParentIdField,     "TEXT NOT NULL"},
        {RelationTypeField, "INTEGER NOT NULL"}
    })
{
}

SettingsTable::SettingsTable()
    : ucf::service::model::DBTableModel(TableName,{
        {KeyField, "TEXT UNIQUE NOT NULL"},
        {ValField, "INTEGER"}
    })
{
}

ScreenshotSettingsTable::ScreenshotSettingsTable()
    : ucf::service::model::DBTableModel(TableName,{
        {SettingsIdentifierField, "TEXT UNIQUE NOT NULL"},
        {OutputDirectoryField, "TEXT"},
        {ImageFormatField, "TEXT"},
        {JpegQualityField, "INTEGER"},
        {CaptureDelayField, "INTEGER"},
        {AddTimestampField, "INTEGER"}
    })
{
}

RecordingSettingsTable::RecordingSettingsTable()
    : ucf::service::model::DBTableModel(TableName,{
        {SettingsIdentifierField, "TEXT UNIQUE NOT NULL"},
        {OutputDirectoryField, "TEXT"},
        {VideoFormatField, "TEXT"},
        {FramesPerSecondField, "INTEGER"},
        {EnableMicrophoneField, "INTEGER"},
        {EnableSystemAudioField, "INTEGER"},
        {MicDeviceIdField, "TEXT"},
        {SystemAudioDeviceIdField, "TEXT"}
    })
{
}

CameraGroupTable::CameraGroupTable()
    : ucf::service::model::DBTableModel(TableName,{
        {NodeIdField,      "TEXT UNIQUE NOT NULL"},
        {DisplayNameField, "TEXT NOT NULL"},
        {NodeStatusField,  "INTEGER"}
    })
{
}

CameraTable::CameraTable()
    : ucf::service::model::DBTableModel(TableName,{
        {NodeIdField,           "TEXT UNIQUE NOT NULL"},
        {DisplayNameField,      "TEXT NOT NULL"},
        {NodeStatusField,       "INTEGER"},
        {SourceTypeField,       "INTEGER"},
        {LocalIndexField,       "INTEGER"},
        {NetworkUrlField,       "TEXT"},
        {NetworkTransportField, "TEXT"},
        {OpenTimeoutMsField,    "INTEGER"},
        {ReadTimeoutMsField,    "INTEGER"}
    })
{
}

CameraDirectoryRelationTable::CameraDirectoryRelationTable()
    : ucf::service::model::DBTableModel(TableName,{
        {ChildIdField,      "TEXT UNIQUE NOT NULL"},
        {ParentIdField,     "TEXT NOT NULL"},
        {RelationTypeField, "INTEGER NOT NULL"}
    })
{
}

}
