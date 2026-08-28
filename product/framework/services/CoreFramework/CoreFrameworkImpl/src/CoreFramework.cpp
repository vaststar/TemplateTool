#include <mutex>
#include <vector>

#include <ucf/services/ServiceDeclaration/IService.h>

#include <ucf/CoreFramework/CoreFrameworkCreator.h>
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

    void setStartupParameters(const std::vector<std::string>& args);
    std::vector<std::string> getStartupParameters() const;
private:
    mutable std::mutex mStateMutex;
    CoreFrameworkState mState;

    mutable std::mutex mStartupParametersMutex;
    std::vector<std::string> mStartupParameters;
};

CoreFramework::DataPrivate::DataPrivate()
    : mState(CoreFrameworkState::InitialState)
{
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

void CoreFramework::DataPrivate::setStartupParameters(const std::vector<std::string>& args)
{
    std::scoped_lock<std::mutex> loc(mStartupParametersMutex);
    mStartupParameters = args;
    CORE_LOG_DEBUG("set startup parameters, count: " << mStartupParameters.size() << ", address:" << this);
}

std::vector<std::string> CoreFramework::DataPrivate::getStartupParameters() const
{
    std::scoped_lock<std::mutex> loc(mStartupParametersMutex);
    return mStartupParameters;
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
std::shared_ptr<ICoreFramework> createCoreFramework()
{
    return std::make_shared<CoreFramework>();
}

CoreFramework::CoreFramework()
    : mDataPrivate(std::make_unique<CoreFramework::DataPrivate>())
{
    CORE_LOG_DEBUG("CoreFramework constructed, address: " << this);
}

CoreFramework::~CoreFramework()
{
    CORE_LOG_DEBUG("CoreFramework destroying, address: " << this);
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

        CORE_LOG_DEBUG("fire onCoreFrameworkExit, address:" << this);
        fireNotification(&ICoreFrameworkCallback::onCoreFrameworkExit);

        CORE_LOG_DEBUG("start exiting CoreFramework, address:" << this);
        deinitServices();

        CORE_LOG_DEBUG("unregister all services, address:" << this);
        unRegisterServices();

        CORE_LOG_DEBUG("finish exiting CoreFramework, address:" << this);
        mDataPrivate->exitFinished();

        CORE_LOG_DEBUG("CoreFramework exited, address:" << this);
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

void CoreFramework::setStartupParameters(const std::vector<std::string>& args)
{
    mDataPrivate->setStartupParameters(args);
}

std::vector<std::string> CoreFramework::getStartupParameters() const
{
    return mDataPrivate->getStartupParameters();
}

void CoreFramework::initServices()
{
    if (!mDataPrivate->isRunnable())
    {
        CORE_LOG_INFO("CoreFramework is not runnable, address:" << this);
        return;
    }

    if (!sortServicesByDependency())
    {
        CORE_LOG_ERROR("failed to resolve service init order, abort initServices, address:" << this);
        return;
    }

    for (const auto& weakService : getAllServices())
    {
        if (auto servicePtr = weakService.lock())
        {
            servicePtr->initComponent();
        }
    }

    fireNotification(&ICoreFrameworkCallback::onServiceInitialized);
}

void CoreFramework::deinitServices()
{
    // The storage is already sorted in initialization order, so shut services
    // down in its exact reverse: a service is torn down before its dependencies.
    auto services = getAllServices();
    for (auto iter = services.rbegin(); iter != services.rend(); ++iter)
    {
        if (auto servicePtr = iter->lock())
        {
            servicePtr->deinitComponent();
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish CoreFramework Logic////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}
