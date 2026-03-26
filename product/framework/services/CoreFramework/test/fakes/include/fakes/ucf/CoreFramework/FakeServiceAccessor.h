#pragma once

#include <trompeloeil.hpp>
#include <ucf/CoreFramework/IServiceAccessor.h>

namespace ucf::framework::fakes{
class FakeServiceAccessor: public virtual IServiceAccessor
{
public:
    MAKE_MOCK0(getAllServices, std::vector<std::weak_ptr<ucf::service::IService>>(), override);
    MAKE_MOCK3(registerServiceInternal, void(std::type_index, ucf::service::IServicePtr, bool), override);
    MAKE_MOCK1(getServiceInternal, ucf::service::IServicePtr(std::type_index), override);
    MAKE_MOCK0(unRegisterServices, void(), override);
};
}