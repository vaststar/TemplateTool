#pragma once

#include <trompeloeil.hpp>
#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>
#include <memory>


namespace fakes{
class SERVICE_TEST_EXPORT FakeCoreFramework : public virtual ucf::framework::ICoreFramework,
                                              public virtual ucf::utilities::NotificationHelper<ucf::framework::ICoreFrameworkCallback>,
                                              public std::enable_shared_from_this<FakeCoreFramework>
{
public:
    FakeCoreFramework();
    //static constexpr bool trompeloeil_movable_mock = true;
    // FakeCoreFramework(const FakeCoreFramework&) = delete;
    // FakeCoreFramework(FakeCoreFramework&&) = delete;
    // FakeCoreFramework& operator=(const FakeCoreFramework&) = delete;
    // FakeCoreFramework& operator=(FakeCoreFramework&&) = delete;
public:
    MAKE_MOCK0(initCoreFramework, void(), override);
    MAKE_MOCK0(exitCoreFramework, void(), override);
    MAKE_MOCK0(initServices, void(), override);
    MAKE_CONST_MOCK0(getName, std::string(), override);
    MAKE_MOCK0(getAllServices, std::vector<std::weak_ptr<ucf::service::IService>>(), override);
    MAKE_MOCK3(registerServiceInternal, void(std::type_index, ucf::service::IServicePtr, bool), override);
    MAKE_MOCK1(getServiceInternal, ucf::service::IServicePtr(std::type_index), override);
    MAKE_MOCK0(unRegisterServices, void(), override);
    // MAKE_MOCK1(registerCallback, void(std::shared_ptr<ucf::framework::ICoreFrameworkCallback>), override);
    // MAKE_MOCK1(unRegisterCallback, void(std::shared_ptr<ucf::framework::ICoreFrameworkCallback>), override);
    
};
}
