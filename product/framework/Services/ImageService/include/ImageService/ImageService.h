#pragma once

#include <memory>
#include <string>
#include "IImageService.h"

class ICoreFramework;
class SERVICE_EXPORT ImageService:public IImageService
{
public:
    ImageService(std::weak_ptr<ICoreFramework> coreFramework);
    ~ImageService();
    virtual std::string getServiceName() const override;
};

