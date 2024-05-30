#pragma once

#include <string>

#include <ucf/ServiceCommonFile/ServiceExport.h>

namespace ucf{
class SERVICE_EXPORT IService
{
public:
    virtual std::string getServiceName() const = 0;
    virtual ~IService() = default;
    virtual void initService() = 0;
};
}