#pragma once

#include <string>
#include <memory>
#include <mutex>

#include <ucf/Services/ServiceExportMacro/ServiceExport.h>

namespace ucf::service{
class SERVICE_EXPORT IService
{
public:
    IService() = default;
    IService(const IService&) = delete;
    IService(IService&&) = delete;
    IService& operator=(const IService&) = delete;
    IService& operator=(IService&&) = delete;
    virtual ~IService() = default;
public:
    [[nodiscard]] virtual std::string getServiceName() const = 0;
    void initComponent();
protected:
    virtual void initService() = 0;
private:
    std::once_flag mInitFlag;
};
}