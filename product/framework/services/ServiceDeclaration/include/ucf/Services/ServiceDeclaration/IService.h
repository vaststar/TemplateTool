#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <vector>
#include <typeindex>

#include <ucf/Services/ServiceExportMacro/ServiceExport.h>

namespace ucf::service{

enum class DependencyKind
{
    // The dependency must be registered; a missing required dependency aborts
    // the whole initialization.
    Required,
    // The dependency is used when present but is not mandatory; a missing
    // optional dependency is ignored and this service still initializes.
    Optional
};

struct ServiceDependency
{
    ServiceDependency(std::type_index dependencyType, DependencyKind dependencyKind)
        : type(dependencyType)
        , kind(dependencyKind)
    {
    }

    std::type_index type;
    DependencyKind kind;
};

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

    [[nodiscard]] virtual std::vector<ServiceDependency> dependencies() const;

    void initComponent();
    void deinitComponent();
protected:
    virtual void initService() = 0;
    virtual void deinitService() = 0;
private:
    std::once_flag mInitFlag;
    std::once_flag mDeinitFlag;
};
}
