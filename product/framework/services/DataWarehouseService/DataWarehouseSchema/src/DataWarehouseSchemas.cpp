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

PersonContactTable::PersonContactTable()
    : ucf::service::model::DBTableModel(TableName,{
        {ContactIdField, "TEXT UNIQUE NOT NULL"},
        {FirstNameField, "TEXT"},
        {LastNameField,  "TEXT"},
        {GenderField,    "INTEGER"},
        {PhoneField,     "TEXT"},
        {EmailField,     "TEXT"}
    })
{
}

GroupContactTable::GroupContactTable()
    : ucf::service::model::DBTableModel(TableName,{
        {GroupIdField,         "TEXT UNIQUE NOT NULL"},
        {GroupNameField,       "TEXT NOT NULL"},
        {GroupTypeField,       "INTEGER NOT NULL"},
        {ContactStatusField,   "INTEGER"}
    })
{
}

DepartmentGroupTable::DepartmentGroupTable()
    : ucf::service::model::DBTableModel(TableName,{
        {GroupIdField,    "TEXT UNIQUE NOT NULL"},
        {ManagerIdField,  "TEXT"},
        {HeadcountField,  "INTEGER"}
    })
{
}

TeamGroupTable::TeamGroupTable()
    : ucf::service::model::DBTableModel(TableName,{
        {GroupIdField,    "TEXT UNIQUE NOT NULL"},
        {TeamLeadIdField, "TEXT"},
        {MissionField,    "TEXT"}
    })
{
}

FolderGroupTable::FolderGroupTable()
    : ucf::service::model::DBTableModel(TableName,{
        {GroupIdField, "TEXT UNIQUE NOT NULL"}
    })
{
}

ContactRelationTable::ContactRelationTable()
    : ucf::service::model::DBTableModel(TableName,{
        // RELATION_ID 为代理主键（UUID）；(CHILD_ID, PARENT_ID, RELATION_TYPE) 由应用层维护业务唯一性。
        {RelationIdField,   "TEXT UNIQUE NOT NULL"},
        {ChildIdField,      "TEXT NOT NULL"},
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
        {RelationIdField,   "TEXT UNIQUE NOT NULL"},
        {ChildIdField,      "TEXT NOT NULL"},
        {ParentIdField,     "TEXT NOT NULL"},
        {RelationTypeField, "INTEGER NOT NULL"}
    })
{
}

}
