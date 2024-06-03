#pragma once

#include <ucf/Utilities/NotificationHelper/INotificationHelper.h>
#include <ucf/CoreFramework/IService.h>
#include <ucf/Services/ContactService/IContactServiceCallback.h>

namespace ucf{

class ICoreFramework;
using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;

namespace model{
    class Contact;
}

class SERVICE_EXPORT IContactService: public IService, 
                                      public virtual ucf::utilities::INotificationHelper<IContactServiceCallback>
{
public:
    virtual void fetchContactList() = 0;
    virtual std::vector<model::Contact> getContactList() const = 0;
    static std::shared_ptr<IContactService> CreateInstance(ICoreFrameworkWPtr coreFramework);
};
}