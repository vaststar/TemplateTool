#pragma once

#include <typeindex>
#include <memory>
#include <vector>

#include <ucf/Services/ServiceCommonFile/ServiceExport.h>

namespace ucf::service {
    class IService;
    using IServicePtr = std::shared_ptr<IService>;
}
namespace ucf::framework{
class SERVICE_EXPORT IServiceAccessor
{
public:
    template<typename T>
    std::weak_ptr<T> getService()
    {
        static_assert(std::is_base_of_v<ucf::service::IService, T> == true);
        return std::weak_ptr(std::dynamic_pointer_cast<T>(getServiceInternal(std::type_index(typeid(T)))));
    }

    virtual std::vector<std::weak_ptr<ucf::service::IService>> getAllServices() = 0;

    template <typename T>
    void registerService(ucf::service::IServicePtr service, bool overrideExisting = true)
    {
        static_assert(std::is_base_of_v<ucf::service::IService, T> == true);
        registerServiceInternal(std::type_index(typeid(T)), std::move(service), overrideExisting);
    }
protected:
    virtual ucf::service::IServicePtr getServiceInternal(std::type_index index) = 0;
    virtual void registerServiceInternal(std::type_index index, ucf::service::IServicePtr service, bool overrideExisting) = 0;
    virtual void unRegisterServices() = 0;
};
}