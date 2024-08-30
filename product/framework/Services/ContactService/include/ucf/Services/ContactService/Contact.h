#pragma once

#include <string>
#include <vector>

#include <ucf/Services/ServiceCommonFile/ServiceExport.h>

namespace ucf::service::model{
class SERVICE_EXPORT IContact
{
public:
    explicit IContact(const std::string& id)
        : mContactId(id)
    {}
    virtual ~IContact() = default;
    std::string getContactId() const{ return mContactId;}
private:
    const std::string mContactId;
};

class SERVICE_EXPORT PersonContact: public IContact
{
public:
    explicit PersonContact(const std::string& id);
    
    const std::string& getContactName() const;
    void setContactName(const std::string& name);
private:
    std::string mContactName;
};

class SERVICE_EXPORT GroupContact: public IContact
{
public:
    explicit GroupContact(const std::string& id);
private:
    std::string mGroupName;
};
}