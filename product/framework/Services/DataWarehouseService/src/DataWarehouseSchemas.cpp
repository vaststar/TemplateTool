#include "DataWarehouseSchemas.h"

namespace db::schema{
UserContactTable::UserContactTable()
    : ucf::utilities::database::DatabaseSchema(TableName,{
        {ContactIdField, "TEXT UNIQUE NOT NULL"},
        {ContactFullNameField, "TEXT NOT NULL"},
        {ContactEmailField, "TEXT"}
    })
{
}

GroupContactTable::GroupContactTable()
    : ucf::utilities::database::DatabaseSchema(TableName,{
        {GroupIdField, "TEXT UNIQUE NOT NULL"},
        {GroupNameField, "TEXT NOT NULL"}
    })
{
}
}