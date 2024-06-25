#pragma once

#include <memory>
#include <string>
#include <ucf/Services/ImageService/IImageService.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}
namespace ucf::service{
class SERVICE_EXPORT ImageService:public IImageService
{
public:
    ImageService(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ~ImageService();
    //IService
    virtual std::string getServiceName() const override;
    virtual void initService() override;
private:
    ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
};
}
