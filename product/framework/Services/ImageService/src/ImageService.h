#pragma once

#include <memory>
#include <string>
#include <ucf/Services/ImageService/IImageService.h>


namespace ucf{
class ICoreFramework;
class SERVICE_EXPORT ImageService:public IImageService
{
public:
    ImageService(std::weak_ptr<ICoreFramework> coreFramework);
    ~ImageService();
    //IService
    virtual std::string getServiceName() const override;
    virtual void initService() override;
    virtual void onCoreFrameworkExit() override;
private:
    std::weak_ptr<ICoreFramework> mCoreFrameworkWPtr;
};
}
