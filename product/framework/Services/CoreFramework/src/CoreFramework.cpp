#include <mutex>

#include <ucf/CoreFramework/IService.h>

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

std::shared_ptr<ICoreFramework> ICoreFramework::CreateInstance()
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

void CoreFramework::initServices()
{
    if (mDataPrivate->isRunnable())
    {
        auto allServices = getAllServices();
        std::for_each(allServices.begin(), allServices.end(), [](std::weak_ptr<ucf::service::IService> service){
            if (auto servicePtr = service.lock())
            {
                servicePtr->initService();
            }
        }); 
        fireNotification(&ICoreFrameworkCallback::OnServiceInitialized);
    }
    else
    {
        CORE_LOG_INFO("CoreFramework is not runnable, address:" << this);
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish CoreFramework Logic////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}