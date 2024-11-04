#pragma once

#include <memory>
#include <string>
#include <ucf/Services/MediaService/IMediaService.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}
namespace ucf::service{
class SERVICE_EXPORT MediaService:public IMediaService
{
public:
    MediaService(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ~MediaService();
    //IService
    virtual std::string getServiceName() const override;

    //MediaService
    virtual void openCamera() override;
protected:
    //IService
    virtual void initService() override;
private:
    ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
};
}
