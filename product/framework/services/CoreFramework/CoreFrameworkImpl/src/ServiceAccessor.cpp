#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <typeindex>
#include <vector>

#include <ucf/Services/ServiceDeclaration/IService.h>

#include "ServiceAccessor.h"
#include "CoreframeworkLogger.h"

namespace ucf::framework{

std::vector<std::weak_ptr<ucf::service::IService>> ServiceAccessor::getAllServices()
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

ucf::service::IServicePtr ServiceAccessor::getServiceInternal(std::type_index index)
{
    std::scoped_lock loc(mDataMutex);
    if (auto iter = findServiceWithoutLock(index); iter != mServices.end())
    {
        return iter->second;
    }
    return {};
}

void ServiceAccessor::registerServiceInternal(std::type_index index, ucf::service::IServicePtr service, bool overrideExisting)
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

void ServiceAccessor::unRegisterServices()
{
    std::scoped_lock loc(mDataMutex);
    while (!mServices.empty())
    {
        mServices.pop_back();
    }
}

std::vector<std::pair<std::type_index, ucf::service::IServicePtr>>::iterator
ServiceAccessor::findServiceWithoutLock(std::type_index index)
{
    return std::find_if(mServices.begin(), mServices.end(),
        [&index](const auto& entry) { return entry.first == index; });
}

bool ServiceAccessor::sortServicesByDependency()
{
    std::scoped_lock loc(mDataMutex);

    std::unordered_set<std::type_index> knownTypes;
    std::unordered_map<std::type_index, int> remainingDependencyCount;
    std::unordered_map<std::type_index, std::vector<std::type_index>> dependentServices;

    for (const auto& [serviceType, servicePtr] : mServices)
    {
        knownTypes.insert(serviceType);
        remainingDependencyCount[serviceType] = 0;
    }

    for (const auto& [serviceType, servicePtr] : mServices)
    {
        for (const auto& dependency : servicePtr->dependencies())
        {
            if (knownTypes.find(dependency.type) == knownTypes.end())
            {
                if (dependency.kind == ucf::service::DependencyKind::Required)
                {
                    CORE_LOG_ERROR("missing required dependency for service: " << servicePtr->getServiceName()
                                   << ", required type: " << dependency.type.name());
                    return false;
                }
                // Optional dependency is absent: ignore it and let this service
                // initialize without that collaborator.
                continue;
            }
            dependentServices[dependency.type].push_back(serviceType);
            ++remainingDependencyCount[serviceType];
        }
    }

    std::vector<std::pair<std::type_index, ucf::service::IServicePtr>> orderedServices;
    orderedServices.reserve(mServices.size());
    std::unordered_set<std::type_index> placedTypes;

    // Rescan in registration order each round so that services with no mutual
    // dependency keep their registration order as a stable tie-break.
    while (orderedServices.size() < mServices.size())
    {
        bool madeProgress = false;
        for (const auto& entry : mServices)
        {
            const auto& serviceType = entry.first;
            if (placedTypes.count(serviceType) > 0)
            {
                continue;
            }
            if (remainingDependencyCount[serviceType] != 0)
            {
                continue;
            }

            orderedServices.push_back(entry);
            placedTypes.insert(serviceType);
            madeProgress = true;

            for (const auto& dependentType : dependentServices[serviceType])
            {
                --remainingDependencyCount[dependentType];
            }
            break;
        }

        if (!madeProgress)
        {
            CORE_LOG_ERROR("cyclic dependency detected among services, sorted "
                           << orderedServices.size() << " of " << mServices.size());
            return false;
        }
    }

    mServices = std::move(orderedServices);
    return true;
}

}
