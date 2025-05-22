#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <map>
#include <initializer_list>
#include <optional>

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
    std::vector<model::PersonContact> getPersonContacts() const;
    std::optional<model::PersonContact> getPersonContact(const std::string& contactId) const;
    void addPersonContact(std::unique_ptr<model::PersonContact>&& contacts);
    void deletePersonContacts(const std::initializer_list<std::string>& contactId);
private:
    mutable std::mutex mContactMutex;
    std::map<std::string, std::unique_ptr<model::PersonContact>> mPersonContacts;
    ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
    // std::shared_ptr<ucf::utilities::database::IDatabaseWrapper> mContactDatabase;
};
}