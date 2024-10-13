#include "DataWarehouseSchemas.h"

namespace db::schema{
UserContactTable::UserContactTable()
    : ucf::service::model::DBTableModel(TableName,{
        {ContactIdField, "TEXT UNIQUE NOT NULL"},
        {ContactFullNameField, "TEXT NOT NULL"},
        {ContactEmailField, "TEXT"}
    })
{
}

GroupContactTable::GroupContactTable()
    : ucf::service::model::DBTableModel(TableName,{
        {GroupIdField, "TEXT UNIQUE NOT NULL"},
        {GroupNameField, "TEXT NOT NULL"}
    })
{
}
}