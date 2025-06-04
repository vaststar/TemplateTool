#pragma once

#include <string>
#include <vector>
#include <mutex>

#include <ucf/Services/ContactService/IContactEntities.h>

namespace ucf::service::model{
class PersonContact: public IPersonContact
{
public:
    explicit PersonContact(const std::string& id);
    ~PersonContact();
    
    virtual std::string getContactId() const override;
    virtual std::string getPersonName() const override;

    void setPersonName(const std::string& name);
private:
    mutable std::mutex mDataMutex;
    const std::string mContactId;
    std::string mPersonName;
};
using PersonContactPtr = std::shared_ptr<PersonContact>;

class GroupContact: public IGroupContact
{
public:
    explicit GroupContact(const std::string& id);
    virtual std::string getContactId() const override;
    virtual std::string getGroupName() const override;
    void setGroupName(const std::string& groupName);
private:
    mutable std::mutex mDataMutex;
    const std::string mContactId;
    std::string mGroupName;
};
using GroupContactPtr = std::shared_ptr<GroupContact>;
}