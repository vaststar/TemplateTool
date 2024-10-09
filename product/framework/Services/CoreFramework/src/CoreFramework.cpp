#include "CoreFramework.h"

#include <mutex>
#include <ucf/Services/ServiceCommonFile/ServiceLogger.h>
#include <ucf/CoreFramework/IService.h>
#include <ucf/DataWarehouse/IDataWarehouse.h>

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
    std::shared_ptr<db::IDataWarehouse> getDataWarehouse();
private:
    mutable std::mutex mStateMutex;
    CoreFrameworkState mState;
    std::shared_ptr<db::IDataWarehouse> mDataWarehouse;
};

CoreFramework::DataPrivate::DataPrivate()
    : mState(CoreFrameworkState::InitialState)
    , mDataWarehouse(db::IDataWarehouse::createDataWarehouse())
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

std::shared_ptr<db::IDataWarehouse> CoreFramework::DataPrivate::getDataWarehouse()
{
    return mDataWarehouse;
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
        mDataPrivate->getDataWarehouse()->initializeDB({db::DBEnum::SHARED_DB, "shared_db.db"});
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

std::shared_ptr<db::IDataWarehouse> CoreFramework::getDataWarehouse()
{
    if (mDataPrivate->isRunnable())
    {
        mDataPrivate->getDataWarehouse();
    }
    else
    {
        CORE_LOG_INFO("CoreFramework is not runnable, address:" << this);
    }
    return nullptr;
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish CoreFramework Logic////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}