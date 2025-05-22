#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <string>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{
namespace model {
    class PersonContact;
}

class ContactManager final
{
public:
    ContactManager(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ~ContactManager();
    ContactManager(const ContactManager&) = delete;
    ContactManager(ContactManager&&) = delete;
    ContactManager& operator=(const ContactManager&) = delete;
    ContactManager& operator=(ContactManager&&) = delete;
public:
    std::vector<model::PersonContact> getPersonContactList() const;
    std::optional<model::PersonContact> getPersonContact(const std::string& contactId) const;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}