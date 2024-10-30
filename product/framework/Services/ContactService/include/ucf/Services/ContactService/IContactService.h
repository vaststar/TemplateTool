#pragma once

#include <functional>
#include <vector>
#include <memory>
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
    virtual std::vector<model::PersonContact> getPersonContactList() const = 0;
    static std::shared_ptr<IContactService> CreateInstance(ucf::framework::ICoreFrameworkWPtr coreFramework);
};
}