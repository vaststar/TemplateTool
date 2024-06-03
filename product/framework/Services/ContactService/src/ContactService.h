#pragma once

#include <memory>

#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/ContactService/IContactService.h>
#include <ucf/Services/ContactService/ContactModel.h>



namespace ucf{
class ContactService: public virtual IContactService, 
                      public virtual ucf::utilities::NotificationHelper<IContactServiceCallback>,
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
}