#pragma once

#include <trompeloeil.hpp>
#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>
#include <memory>

#include <fakes/ucf/CoreFramework/FakeServiceAccessor.h>

namespace ucf::framework::fakes{
class SERVICE_EXPORT FakeCoreFramework : public virtual ucf::framework::ICoreFramework,
                          public virtual FakeServiceAccessor,
                          public virtual ucf::utilities::NotificationHelper<ucf::framework::ICoreFrameworkCallback>,
                          public std::enable_shared_from_this<FakeCoreFramework>
{
public:
    FakeCoreFramework();
    MAKE_MOCK0(initCoreFramework, void(), override);
    MAKE_MOCK0(exitCoreFramework, void(), override);
    MAKE_MOCK0(initServices, void(), override);
    MAKE_CONST_MOCK0(getName, std::string(), override);
};
}