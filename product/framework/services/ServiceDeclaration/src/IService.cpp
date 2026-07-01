#include <ucf/Services/ServiceDeclaration/IService.h>

namespace ucf::service{

void IService::initComponent()
{
    std::call_once(mInitFlag, [this](){
        initService();
    });
}

void IService::deinitComponent()
{
    std::call_once(mDeinitFlag, [this](){
        deinitService();
    });
}

std::vector<ServiceDependency> IService::dependencies() const
{
    return {};
}

}
