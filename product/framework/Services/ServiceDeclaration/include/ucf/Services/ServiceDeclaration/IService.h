#pragma once

#include <string>
#include <memory>
#include <mutex>

#include <ucf/Services/ServiceExportMacro/ServiceExport.h>

namespace ucf::service{
class SERVICE_EXPORT IService
{
public:
    virtual std::string getServiceName() const = 0;
    virtual ~IService() = default;
    void initComponent();
protected:
    virtual void initService() = 0;
private:
    std::once_flag mInitFlag;
};
}