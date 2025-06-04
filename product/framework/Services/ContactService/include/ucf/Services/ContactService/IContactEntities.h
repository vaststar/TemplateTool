#pragma once

#include <string>
#include <vector>

#include <ucf/Services/ServiceExportMacro/ServiceExport.h>

namespace ucf::service::model{

class SERVICE_EXPORT IContact
{
public:
    virtual ~IContact() = default;

    // Returns the unique identifier of the contact, only used for identification purposes.
    virtual std::string getContactId() const = 0;
};
using IContactPtr = std::shared_ptr<IContact>;

// Represents a contact for a person, which can be used to store personal information.
class SERVICE_EXPORT IPersonContact: public IContact
{
public:
    virtual std::string getPersonName() const = 0;
};
using IPersonContactPtr = std::shared_ptr<IPersonContact>;

// Represents a contact for a group, which can be used to store group information.
class SERVICE_EXPORT IGroupContact: public IContact
{
public:
    virtual std::string getGroupName() const = 0;
};
using IGroupContactPtr = std::shared_ptr<IGroupContact>;

}
