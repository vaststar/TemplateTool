#pragma once
#include "CoreFramework/IService.h"
#include <Utilities/NotificationHelper/NotificationHelper.h>


class ICoreFramework;
using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;

namespace model{
    class Contact;
}

class IContactServiceCallback
{
public:
virtual ~ IContactServiceCallback() = default;
virtual void OnContactListAvailable(const std::vector<model::Contact>& contactList) = 0;
};

class SERVICE_EXPORT IContactService: public IService, public NotificationHelper<IContactServiceCallback>
{
public:
    virtual void fetchContactList() = 0;
    virtual std::vector<model::Contact> getContactList() const = 0;
    static std::shared_ptr<IContactService> CreateInstance(ICoreFrameworkWPtr coreFramework);
};