#pragma once

#include <ucf/Services/ServiceExportMacro/ServiceExport.h>
#include <ucf/Services/DataWarehouseService/DataBaseTableModel.h>
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
}