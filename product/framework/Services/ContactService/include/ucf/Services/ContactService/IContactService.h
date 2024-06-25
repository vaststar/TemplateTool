#pragma once

#include <ucf/Utilities/NotificationHelper/INotificationHelper.h>
#include <ucf/CoreFramework/IService.h>
#include <ucf/Services/ContactService/IContactServiceCallback.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{
namespace model{
    class Contact;
}

class SERVICE_EXPORT IContactService: public IService, 
                                      public virtual ucf::utilities::INotificationHelper<IContactServiceCallback>
{
public:
    virtual void fetchContactList() = 0;
    virtual std::vector<model::Contact> getContactList() const = 0;
    static std::shared_ptr<IContactService> CreateInstance(ucf::framework::ICoreFrameworkWPtr coreFramework);
};
}