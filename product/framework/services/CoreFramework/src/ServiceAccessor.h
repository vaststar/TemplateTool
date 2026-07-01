#pragma once

#include <typeindex>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

#include <ucf/CoreFramework/IServiceAccessor.h>

namespace ucf::service{
    class IService;
    using IServicePtr = std::shared_ptr<IService>;
}

namespace ucf::framework{
class ServiceAccessor: public virtual IServiceAccessor
{
public:
    ServiceAccessor() = default;
    ServiceAccessor(const ServiceAccessor&) = delete;
    ServiceAccessor(ServiceAccessor&&) = delete;
    ServiceAccessor& operator=(const ServiceAccessor&) = delete;
    ServiceAccessor& operator=(ServiceAccessor&&) = delete;
    virtual ~ServiceAccessor() = default;
public:
    virtual std::vector<std::weak_ptr<ucf::service::IService>> getAllServices() override;

protected:
    // Reorders the internal storage into a deterministic, dependency-respecting
    // order so that getAllServices() reflects the initialization order.
    // Returns false when a cyclic or missing dependency is detected.
    bool sortServicesByDependency();

    virtual ucf::service::IServicePtr getServiceInternal(std::type_index index) override;
    virtual void registerServiceInternal(std::type_index index, ucf::service::IServicePtr service, bool overrideExisting) override;
    virtual void unRegisterServices() override;

private:
    std::vector<std::pair<std::type_index, ucf::service::IServicePtr>>::iterator findServiceWithoutLock(std::type_index index);

private:
    std::mutex mDataMutex;
    std::vector<std::pair<std::type_index, ucf::service::IServicePtr>> mServices;
};
}
