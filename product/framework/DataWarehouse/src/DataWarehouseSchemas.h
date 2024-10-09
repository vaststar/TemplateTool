#pragma once

#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DataBaseSchema.h>
namespace db{
struct UserContactTable: public ucf::utilities::database::DataBaseSchema
{
    UserContactTable();
    static constexpr auto TableName = "UserContact";
    static constexpr auto ContactIdField = "CONTACT_ID";
    static constexpr auto ContactFullNameField = "CONTACT_FULL_NAME";
    static constexpr auto ContactEmailField = "CONTACT_EMAIL";
};

struct GroupContactTable: public ucf::utilities::database::DataBaseSchema
{
    GroupContactTable();
    static constexpr auto TableName = "GroupContact";
    static constexpr auto GroupIdField = "GROUP_ID";
    static constexpr auto GroupNameField = "GROUP_NAME";
};
}