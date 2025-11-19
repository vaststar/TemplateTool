#pragma once

#include <memory>
#include <string>
#include <ucf/Services/ImageService/IImageService.h>
#include <ucf/CoreFramework/CoreFrameworkCallbackDefault.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}
namespace ucf::service{
class SERVICE_EXPORT ImageService:public IImageService, public ucf::framework::CoreFrameworkCallbackDefault
{
public:
    ImageService(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ImageService(const ImageService&) = delete;
    ImageService(ImageService&&) = delete;
    ImageService& operator=(const ImageService&) = delete;
    ImageService& operator=(ImageService&&) = delete;
    ~ImageService();
public:
    //IService
    [[nodiscard]] virtual std::string getServiceName() const override;
protected:
    //IService
    virtual void initService() override;

    
    virtual void onServiceInitialized() override{};
private:
    ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
};
}
