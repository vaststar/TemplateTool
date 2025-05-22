#pragma once

#include <memory>

#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>

#include <ucf/CoreFramework/CoreFrameworkCallbackDefault.h>
#include <ucf/Services/ContactService/IContactService.h>
#include <ucf/Services/ContactService/Contact.h>


namespace ucf::service{
class ContactService final: public virtual IContactService, 
                      public virtual ucf::utilities::NotificationHelper<IContactServiceCallback>,
                      public ucf::framework::CoreFrameworkCallbackDefault,
                      public std::enable_shared_from_this<ContactService>
{
public:
    explicit ContactService(ucf::framework::ICoreFrameworkWPtr coreFramework);
    virtual ~ContactService();
    ContactService(const ContactService&) = delete;
    ContactService(ContactService&&) = delete;
    ContactService& operator=(const ContactService&) = delete;
    ContactService& operator=(ContactService&&) = delete;

    //IContactService
    // virtual void fetchContactList(std::function<void(const std::vector<model::Contact>&)> contactListCallback) override;
    virtual std::vector<model::PersonContact> getPersonContactList() const override;
    virtual std::optional<model::PersonContact> getPersonContact(const std::string& contactId) const override;

    //IService
    virtual std::string getServiceName() const override;

    //ICoreFrameworkCallback
    virtual void onServiceInitialized() override;
    virtual void onCoreFrameworkExit() override;
protected:
    //IService
    virtual void initService() override;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}