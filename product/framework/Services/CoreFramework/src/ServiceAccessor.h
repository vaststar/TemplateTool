#pragma once

#include <typeindex>
#include <memory>
#include <mutex>
#include <map>
#include <utility>

#include <ucf/CoreFramework/IServiceAccessor.h>

namespace ucf{

class IService;
using IServicePtr = std::shared_ptr<IService>;

class ServiceAccessor: public virtual IServiceAccessor
{
public:
    virtual std::vector<std::weak_ptr<IService>> getAllServices() override
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

protected:
    virtual IServicePtr getServiceInternal(std::type_index index) override
    {
        std::scoped_lock loc(mDataMutex);
        if (auto iter = mServices.find(index); iter != mServices.end())
        {
            return (*iter).second;
        }
        return {};
    }

    virtual void registerServiceInternal(std::type_index index, IServicePtr service, bool overrideExisting) override
    {
        std::scoped_lock loc(mDataMutex);
        if (overrideExisting || !mServices.contains(index))
        {
            mServices[index] = service;
        }
    }

    virtual void unRegisterServices() override
    {
        std::map<std::type_index, IServicePtr> emptyServices;
        std::mutex mDataMutex;
        std::swap(mServices, emptyServices);
    }
private:
    std::mutex mDataMutex;
    std::map<std::type_index, IServicePtr> mServices;
};
}