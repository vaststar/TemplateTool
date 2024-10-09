#include "DataWarehouseSchemas.h"

namespace db{
UserContactTable::UserContactTable()
    : ucf::utilities::database::DataBaseSchema(TableName,{
        {ContactIdField, "TEXT UNIQUE NOT NULL"},
        {ContactFullNameField, "TEXT NOT NULL"},
        {ContactEmailField, "TEXT"}
    })
{
}

GroupContactTable::GroupContactTable()
    : ucf::utilities::database::DataBaseSchema(TableName,{
        {GroupIdField, "TEXT UNIQUE NOT NULL"},
        {GroupNameField, "TEXT NOT NULL"}
    })
{
}
}