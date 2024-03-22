#include "ContactService/ContactService.h"
#include "ServiceCommonFile/ServiceLogger.h"


std::shared_ptr<IContactService> IContactService::CreateInstance(ICoreFrameworkWPtr coreFramework)
{
    return std::make_shared<ContactService>(coreFramework);
}

ContactService::ContactService(ICoreFrameworkWPtr coreFramework)
    :mCoreFrameworkWPtr(coreFramework)
{
    SERVICE_LOG_DEBUG("Create ContactService");
}

std::string ContactService::getServiceName() const
{
    return "ContactService";
}

void ContactService::fetchContactList()
{
    SERVICE_LOG_DEBUG("start fetchContactList");
    fireNotification(&IContactServiceCallback::OnContactListAvailable);
    SERVICE_LOG_DEBUG("finish fetchContactList");
}