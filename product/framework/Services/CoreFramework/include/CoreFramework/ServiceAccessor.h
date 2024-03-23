#pragma once

#include <typeindex>
#include <memory>
#include <mutex>
#include <map>

#include "ServiceCommonFile/ServiceExport.h"

class IService;
using IServicePtr = std::shared_ptr<IService>;
class SERVICE_EXPORT ServiceAccessor
{
public:
    template<typename T>
    std::weak_ptr<IService> getService()
    {
        static_assert(std::is_base_of_v<IService, T> == true);
        std::scoped_lock loc(mDataMutex);
        if (auto iter = mServices.find(std::type_index(typeid(T))); iter != mServices.end())
        {
            return std::weak_ptr((*iter).second);
        }
        return {};
    }

    template<typename T>
    void registerService(IServicePtr service, bool overrideExisting = true)
    {
        static_assert(std::is_base_of_v<IService, T> == true);
        std::scoped_lock loc(mDataMutex);
        if (auto index = std::type_index(typeid(T)); overrideExisting || !mServices.contains(index))
        {
            mServices[index] = service;
        }
    }

    std::vector<std::weak_ptr<IService>> getAllServices()
    {
        std::vector<std::weak_ptr<IService>> services;
        {
            std::scoped_lock loc(mDataMutex);
            for(auto [_, service] : mServices)
            {
                services.push_back(std::weak_ptr(service));
            }
        }
        return services;
    }

public:
    virtual ~ServiceAccessor() = default;
private:
    std::mutex mDataMutex;
    std::map<std::type_index, IServicePtr> mServices;
};