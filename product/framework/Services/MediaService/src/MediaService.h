#pragma once

#include <memory>
#include <string>
#include <ucf/Services/MediaService/IMediaService.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}
namespace ucf::service{
class SERVICE_EXPORT MediaService final: public IMediaService
{
public:
    MediaService(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ~MediaService();
    //IService
    virtual std::string getServiceName() const override;

    //MediaService
    virtual std::string openCamera(int cameraNum) override;
    virtual void releaseCamera(const std::string& cameraId) override;
    virtual std::vector<std::string> getOpenedCameras() const override;
    virtual std::optional<model::Image> readImageData(const std::string& cameraId) override;
protected:
    //IService
    virtual void initService() override;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
    ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
};
}
