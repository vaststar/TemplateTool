#include "ContactService/ContactService.h"
#include "CoreFramework/ICoreFramework.h"
#include "ServiceCommonFile/ServiceLogger.h"


std::shared_ptr<IContactService> IContactService::CreateInstance(ICoreFrameworkWPtr coreFramework)
{
    return std::make_shared<ContactService>(coreFramework);
}

ContactService::ContactService(ICoreFrameworkWPtr coreFramework)
    :mCoreFrameworkWPtr(coreFramework)
{
    SERVICE_LOG_DEBUG("Create ContactService, address:" << this);
}

void ContactService::initService()
{
    mContactModelPtr = std::make_unique<model::ContactModel>(mCoreFrameworkWPtr);
    if (auto coreFramework = mCoreFrameworkWPtr.lock())
    {
        coreFramework->registerCallback(shared_from_this());
    }
}

std::string ContactService::getServiceName() const
{
    return "ContactService";
}

void ContactService::OnDataBaseInitialized()
{

}

void ContactService::fetchContactList()
{
    SERVICE_LOG_DEBUG("start fetchContactList");
    std::vector<model::Contact> contactList;
    if (mContactModelPtr)
    {
        contactList = mContactModelPtr->getContacts();
    }
    fireNotification(&IContactServiceCallback::OnContactListAvailable, contactList);
    SERVICE_LOG_DEBUG("finish fetchContactList");
}

std::vector<model::Contact> ContactService::getContactList() const
{
    if (mContactModelPtr)
    {
        return mContactModelPtr->getContacts();
    }
    return {};
}