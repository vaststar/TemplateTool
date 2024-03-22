#pragma once
#include "ContactService/IContactService.h"
#include "CoreFramework/ICoreFramework.h"

class ContactService: public IContactService, 
                      public std::enable_shared_from_this<ContactService>
{
public:
    explicit ContactService(ICoreFrameworkWPtr coreFramework);

    virtual std::string getServiceName() const override;
    virtual void fetchContactList() override;
private:
    const ICoreFrameworkWPtr mCoreFrameworkWPtr;
};