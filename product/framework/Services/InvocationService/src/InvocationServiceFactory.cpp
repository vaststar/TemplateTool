#include "InvocationService.h"

namespace ucf::service{
std::shared_ptr<IInvocationService> IInvocationService::createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework)
{
    return std::make_shared<InvocationService>(coreFramework);
}
}
