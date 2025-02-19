#pragma once

#include <string>
#include <vector>
#include <mutex>

#include <ucf/Services/ServiceExportMacro/ServiceExport.h>
#include <ucf/Services/ContactService/Contact.h>


// namespace ucf::utilities::database
// {
//     class IDatabaseWrapper;
// }


namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}
namespace ucf::service{

class ContactModel
{
public:
    explicit ContactModel(ucf::framework::ICoreFrameworkWPtr coreFramework);
    
    ContactModel(const ContactModel&) = delete;
    ContactModel(ContactModel&&) = delete;
    ContactModel& operator=(const ContactModel&) = delete;
    ContactModel& operator=(ContactModel&&) = delete;

public:
    void initDatabase();
    const std::vector<model::PersonContact>& getContacts() const;
    void setContacts(const std::vector<model::PersonContact>& contacts);
    void deleteContacts(const std::string& contactId);
    void addOrUpdateContact(const model::PersonContact& contact);
private:
    mutable std::mutex mContactMutex;
    std::vector<model::PersonContact> mContacts;
    ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
    // std::shared_ptr<ucf::utilities::database::IDatabaseWrapper> mContactDatabase;
};
}