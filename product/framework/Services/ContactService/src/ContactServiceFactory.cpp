#include "ContactService.h"

namespace ucf::service{
std::shared_ptr<IContactService> IContactService::createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework)
{
    return std::make_shared<ContactService>(coreFramework);
}
}