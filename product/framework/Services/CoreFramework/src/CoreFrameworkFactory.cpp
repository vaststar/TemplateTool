
#include "CoreFramework.h"


namespace ucf::framework{

    
std::shared_ptr<ICoreFramework> ICoreFramework::createInstance()
{
    return std::make_shared<CoreFramework>();
}
}