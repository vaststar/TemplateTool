#pragma once

#include <memory>
#include <vector>


namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{
namespace model {
    class Contact;
}
class ContactModel;

class ContactManager final
{
public:
    ContactManager(ucf::framework::ICoreFrameworkWPtr coreFramework, std::shared_ptr<ContactModel> contactModel);
    ~ContactManager();
    ContactManager(const ContactManager&) = delete;
    ContactManager(ContactManager&&) = delete;
    ContactManager& operator=(const ContactManager&) = delete;
    ContactManager& operator=(ContactManager&&) = delete;
public:
    std::vector<model::Contact> getContactList() const;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}