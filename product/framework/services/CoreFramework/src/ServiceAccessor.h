#pragma once

#include <typeindex>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>
#include <algorithm>

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
    virtual std::vector<std::weak_ptr<ucf::service::IService>> getAllServices() override
    {
        std::vector<std::weak_ptr<ucf::service::IService>> services;
        {
            std::scoped_lock loc(mDataMutex);
            services.reserve(mServices.size());
            for (const auto& [index, service] : mServices)
            {
                services.push_back(std::weak_ptr(service));
            }
        }
        return services;
    }

    std::vector<std::pair<std::type_index, ucf::service::IServicePtr>> getAllServicesWithType()
    {
        std::scoped_lock loc(mDataMutex);
        return mServices;
    }

protected:
    virtual ucf::service::IServicePtr getServiceInternal(std::type_index index) override
    {
        std::scoped_lock loc(mDataMutex);
        if (auto iter = findServiceWithoutLock(index); iter != mServices.end())
        {
            return iter->second;
        }
        return {};
    }

    virtual void registerServiceInternal(std::type_index index, ucf::service::IServicePtr service, bool overrideExisting) override
    {
        std::scoped_lock loc(mDataMutex);
        if (auto iter = findServiceWithoutLock(index); iter != mServices.end())
        {
            if (overrideExisting)
            {
                iter->second = std::move(service);
            }
        }
        else
        {
            mServices.emplace_back(index, std::move(service));
        }
    }

    virtual void unRegisterServices() override
    {
        std::scoped_lock loc(mDataMutex);
        while (!mServices.empty())
        {
            mServices.pop_back();
        }
    }
private:
    std::vector<std::pair<std::type_index, ucf::service::IServicePtr>>::iterator findServiceWithoutLock(std::type_index index)
    {
        return std::find_if(mServices.begin(), mServices.end(),
            [&index](const auto& entry) { return entry.first == index; });
    }
private:
    std::mutex mDataMutex;
    std::vector<std::pair<std::type_index, ucf::service::IServicePtr>> mServices;
};
}
