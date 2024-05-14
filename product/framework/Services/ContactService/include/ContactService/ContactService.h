#pragma once
#include "CoreFramework/ICoreFramework.h"
#include "ContactService/IContactService.h"
#include "ContactService/ContactModel.h"

#include <memory>


class ContactService: public IContactService, 
                      public ICoreFrameworkCallback,
                      public std::enable_shared_from_this<ContactService>
{
public:
    explicit ContactService(ICoreFrameworkWPtr coreFramework);

    virtual void fetchContactList() override;
    virtual std::vector<model::Contact> getContactList() const override;

    //IService
    virtual void initService() override;
    virtual std::string getServiceName() const override;

    //ICoreFrameworkCallback
    virtual void OnDataBaseInitialized() override;
    virtual void OnServiceInitialized() override;
private:
    std::weak_ptr<ICoreFramework> mCoreFrameworkWPtr;
    std::unique_ptr<model::ContactModel>  mContactModelPtr;
};