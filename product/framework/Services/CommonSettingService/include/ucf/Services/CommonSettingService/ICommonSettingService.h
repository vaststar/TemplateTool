#pragma once

#include <string>
#include <memory>
#include <vector>

// #include <ucf/CoreFramework/IService.h>
#include <ucf/Services/ServiceDeclaration/IService.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{

class SERVICE_EXPORT ICommonSettingService: public IService
{
public:
    static std::shared_ptr<ICommonSettingService> CreateInstance(ucf::framework::ICoreFrameworkWPtr coreFramework);
};
}