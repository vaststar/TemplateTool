#include <ucf/Services/ServiceDeclaration/IService.h>

namespace ucf::service{

void IService::initComponent()
{
    std::call_once(mInitFlag, [this](){
        initService();
    });
}
}