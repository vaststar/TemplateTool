#pragma once

#include <functional>
#include <vector>
#include <memory>
#include <optional>

#include <ucf/Utilities/NotificationHelper/INotificationHelper.h>
#include <ucf/Services/ServiceDeclaration/IService.h>
#include <ucf/Services/ContactService/IContactServiceCallback.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{
namespace model{
    class IPersonContact;
    using IPersonContactPtr = std::shared_ptr<IPersonContact>;
}

class SERVICE_EXPORT IContactService: public IService, public virtual ucf::utilities::INotificationHelper<IContactServiceCallback>
{
public:
    IContactService() = default;
    IContactService(const IContactService&) = delete;
    IContactService(IContactService&&) = delete;
    IContactService& operator=(const IContactService&) = delete;
    IContactService& operator=(IContactService&&) = delete;
    virtual ~IContactService() = default;
public:
    // virtual void fetchContactList(std::function<void(const std::vector<model::Contact>&)> contactListCallback) = 0;
    /**
     * @brief get Full PersonContact List
     * @return std::vector<model::IPersonContactPtr> - List of person contacts
     * @note This function retrieves the full list of person contacts. careful when using it, as it may be a large list.
     */
    [[nodiscard]] virtual std::vector<model::IPersonContactPtr> getPersonContactList() const = 0;

    /**
     * @brief get PersonContact by ID
     * @param contactId - ID of the contact to retrieve
     * @return model::IPersonContactPtr - The person contact if found, otherwise nulptr
     */
    [[nodiscard]] virtual model::IPersonContactPtr getPersonContact(const std::string& contactId) const = 0;
    static std::shared_ptr<IContactService> createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework);
};
}