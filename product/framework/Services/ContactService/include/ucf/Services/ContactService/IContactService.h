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
    class PersonContact;
}

class SERVICE_EXPORT IContactService: public IService, 
                                      public virtual ucf::utilities::INotificationHelper<IContactServiceCallback>
{
public:
    // virtual void fetchContactList(std::function<void(const std::vector<model::Contact>&)> contactListCallback) = 0;
    /**
     * @brief get Full PersonContact List
     * @return std::vector<model::PersonContact> - List of person contacts
     * @note This function retrieves the full list of person contacts. careful when using it, as it may be a large list.
     */
    virtual std::vector<model::PersonContact> getPersonContactList() const = 0;

    /**
     * @brief get PersonContact by ID
     * @param contactId - ID of the contact to retrieve
     * @return std::optional<model::PersonContact> - The person contact if found, otherwise an empty optional
     */
    virtual std::optional<model::PersonContact> getPersonContact(const std::string& contactId) const = 0;
    static std::shared_ptr<IContactService> createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework);
};
}