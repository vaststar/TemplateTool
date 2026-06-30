#include <mutex>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <typeindex>



#include <ucf/Services/ServiceDeclaration/IService.h>

#include "CoreFramework.h"
#include "CoreframeworkLogger.h"

namespace ucf::framework{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

class CoreFramework::DataPrivate
{
public:
    enum class CoreFrameworkState{
        InitialState,
        InitializedState,
        AboutExitingState,
        FinishExitingState
    };
public:
    DataPrivate();
    bool isRunnable() const;
    bool isInitializable() const;
    void initialize();
    void onExiting();
    void exitFinished();
private:
    mutable std::mutex mStateMutex;
    CoreFrameworkState mState;
};

CoreFramework::DataPrivate::DataPrivate()
    : mState(CoreFrameworkState::InitialState)
{
    CORE_LOG_DEBUG("create CoreFramework::DataPrivate, address:" << this);
}

bool CoreFramework::DataPrivate::isRunnable() const
{
    std::scoped_lock<std::mutex> loc(mStateMutex);
    return CoreFrameworkState::InitializedState == mState;
}

bool CoreFramework::DataPrivate::isInitializable() const
{
    std::scoped_lock<std::mutex> loc(mStateMutex);
    return CoreFrameworkState::InitialState == mState;
}

void CoreFramework::DataPrivate::onExiting()
{
    std::scoped_lock<std::mutex> loc(mStateMutex);
    mState = CoreFrameworkState::AboutExitingState;
    CORE_LOG_DEBUG("about exiting coreframework, address:" << this);
}

void CoreFramework::DataPrivate::exitFinished()
{
    std::scoped_lock<std::mutex> loc(mStateMutex);
    mState = CoreFrameworkState::FinishExitingState;
    CORE_LOG_DEBUG("finish exiting coreframework, address:" << this);
}

void CoreFramework::DataPrivate::initialize()
{
    std::scoped_lock<std::mutex> loc(mStateMutex);
    if (CoreFrameworkState::InitialState == mState)
    {
        mState = CoreFrameworkState::InitializedState;
        CORE_LOG_DEBUG("initialize coreframework, address:" << this);
    }
    else
    {
        CORE_LOG_DEBUG("duplicate initialize coreframework, address:" << this);
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start CoreFramework Logic////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ICoreFramework> ICoreFramework::createInstance()
{
    return std::make_shared<CoreFramework>();
}

CoreFramework::CoreFramework()
    : mDataPrivate(std::make_unique<CoreFramework::DataPrivate>())
{
    CORE_LOG_DEBUG("create CoreFramework, address:" << this);
}

CoreFramework::~CoreFramework()
{
    CORE_LOG_DEBUG("delete CoreFramework, address:" << this);
}

void CoreFramework::initCoreFramework()
{
    if (mDataPrivate->isInitializable())
    {
        CORE_LOG_DEBUG("about initialize CoreFramework, address:" << this);
        mDataPrivate->initialize();
        CORE_LOG_DEBUG("finish initialize CoreFramework, address:" << this);
    }
    else
    {
        CORE_LOG_INFO("CoreFramework is not isInitializable, address:" << this);
    }
}

void CoreFramework::exitCoreFramework()
{
    if (mDataPrivate->isRunnable())
    {
        CORE_LOG_DEBUG("about exiting CoreFramework, address:" << this);
        mDataPrivate->onExiting();
        fireNotification(&ICoreFrameworkCallback::onCoreFrameworkExit);
        unRegisterServices();
        mDataPrivate->exitFinished();
        CORE_LOG_DEBUG("finish exiting CoreFramework, address:" << this);
    }
    else
    {
        CORE_LOG_INFO("CoreFramework is not runnable, address:" << this);
    }
}

std::string CoreFramework::getName() const
{
    return "CoreFramework";
}

std::vector<ucf::service::IServicePtr> CoreFramework::buildInitOrder()
{
    auto registeredServices = getAllServicesWithType();

    std::unordered_map<std::type_index, ucf::service::IServicePtr> serviceByType;
    std::unordered_map<std::type_index, int> remainingDependencyCount;
    std::unordered_map<std::type_index, std::vector<std::type_index>> dependentServices;

    for (const auto& [serviceType, servicePtr] : registeredServices)
    {
        serviceByType[serviceType] = servicePtr;
        remainingDependencyCount[serviceType] = 0;
    }

    for (const auto& [serviceType, servicePtr] : registeredServices)
    {
        for (const auto& dependencyType : servicePtr->dependencies())
        {
            if (serviceByType.find(dependencyType) == serviceByType.end())
            {
                CORE_LOG_ERROR("missing dependency for service: " << servicePtr->getServiceName()
                               << ", required type: " << dependencyType.name());
                return {};
            }
            dependentServices[dependencyType].push_back(serviceType);
            ++remainingDependencyCount[serviceType];
        }
    }

    std::vector<ucf::service::IServicePtr> orderedServices;
    std::unordered_set<std::type_index> initializedTypes;

    // Rescan in registration order each round so that services with no mutual
    // dependency keep their registration order as a stable tie-break.
    while (orderedServices.size() < registeredServices.size())
    {
        bool madeProgress = false;
        for (const auto& [serviceType, servicePtr] : registeredServices)
        {
            if (initializedTypes.count(serviceType) > 0)
            {
                continue;
            }
            if (remainingDependencyCount[serviceType] != 0)
            {
                continue;
            }

            orderedServices.push_back(servicePtr);
            initializedTypes.insert(serviceType);
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
                           << orderedServices.size() << " of " << registeredServices.size());
            return {};
        }
    }

    return orderedServices;
}

void CoreFramework::initServices()
{
    if (!mDataPrivate->isRunnable())
    {
        CORE_LOG_INFO("CoreFramework is not runnable, address:" << this);
        return;
    }

    auto orderedServices = buildInitOrder();
    if (orderedServices.empty())
    {
        CORE_LOG_ERROR("service init order is empty, abort initServices, address:" << this);
        return;
    }

    for (const auto& servicePtr : orderedServices)
    {
        CORE_LOG_INFO("start init service: " << servicePtr->getServiceName());
        servicePtr->initComponent();
        CORE_LOG_INFO("finish init service: " << servicePtr->getServiceName());
    }

    fireNotification(&ICoreFrameworkCallback::onServiceInitialized);
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish CoreFramework Logic////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}
