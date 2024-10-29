#pragma once

#include <string>
#include <memory>

#include <ucf/Services/ServiceCommonFile/ServiceExport.h>

namespace ucf::service{
class SERVICE_EXPORT IService
{
public:
    virtual std::string getServiceName() const = 0;
    virtual ~IService() = default;
    virtual void initService() = 0;
// public:
//     void initComponent();

};
}