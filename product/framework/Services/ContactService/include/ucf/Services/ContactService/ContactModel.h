#pragma once

#include <string>
#include <vector>
#include <mutex>

#include <ucf/Services/ServiceCommonFile/ServiceExport.h>


namespace ucf{
class ICoreFramework;

namespace model{
class SERVICE_EXPORT Contact
{
public:
    Contact(const std::string& id, const std::string& name);
    const std::string& getContactId() const;
    
    const std::string& getContactName() const;
    void setContactName(const std::string& name);

    const std::vector<std::string>& getTags() const;
    void setTags(const std::vector<std::string>& tags);
    void addTags(const std::vector<std::string>& tags);
    void removeTags(const std::vector<std::string>& tags);

    bool operator==(const Contact& other);
private:
    std::string mContactId;
    std::string mContactName;
    std::vector<std::string> mContactTags;
};

class SERVICE_EXPORT ContactModel
{
public:
    ContactModel(std::weak_ptr<ICoreFramework> coreFramework);
    const std::vector<Contact>& getContacts() const;
    void setContacts(const std::vector<Contact>& contacts);
    void deleteContacts(const std::string& contactId);
    void addOrUpdateContact(const Contact& contact);

private:
    mutable std::mutex mContactMutex;
    std::vector<Contact> mContacts;
    std::weak_ptr<ICoreFramework> mCoreFrameworkWPtr;
};
}
}