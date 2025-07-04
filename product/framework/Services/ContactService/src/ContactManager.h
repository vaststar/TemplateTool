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
    class IPersonContact;
    using IPersonContactPtr = std::shared_ptr<IPersonContact>;
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
    std::vector<model::IPersonContactPtr> getPersonContactList() const;
    model::IPersonContactPtr getPersonContact(const std::string& contactId) const;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}