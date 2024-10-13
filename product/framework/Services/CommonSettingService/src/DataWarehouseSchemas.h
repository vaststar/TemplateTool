#pragma once

#include <ucf/Services/DataWarehouseService/DataBaseModel.h>
namespace db::schema{
struct UserContactTable: public ucf::service::model::DBTableModel
{
    UserContactTable();
    static constexpr auto TableName = "UserContact";
    static constexpr auto ContactIdField = "CONTACT_ID";
    static constexpr auto ContactFullNameField = "CONTACT_FULL_NAME";
    static constexpr auto ContactEmailField = "CONTACT_EMAIL";
};

struct GroupContactTable: public ucf::service::model::DBTableModel
{
    GroupContactTable();
    static constexpr auto TableName = "GroupContact";
    static constexpr auto GroupIdField = "GROUP_ID";
    static constexpr auto GroupNameField = "GROUP_NAME";
};
}