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
    std::scoped_lock<std::mutex> lock(mStateMutex);
    mState = CoreFrameworkState::AboutExitingState;

    CORE_LOG_DEBUG(
        "CoreFramework internal state changed: "
        "InitializedState -> AboutExitingState"
        ", dataAddress: "
        << this);
}

void CoreFramework::DataPrivate::exitFinished()
{
    std::scoped_lock<std::mutex> lock(mStateMutex);
    mState = CoreFrameworkState::FinishExitingState;

    CORE_LOG_DEBUG(
        "CoreFramework internal state changed: "
        "AboutExitingState -> FinishExitingState"
        ", dataAddress: "
        << this);
}

void CoreFramework::DataPrivate::initialize()
{
    std::scoped_lock<std::mutex> lock(mStateMutex);
    if (CoreFrameworkState::InitialState == mState)
    {
        mState = CoreFrameworkState::InitializedState;

        CORE_LOG_DEBUG(
            "CoreFramework internal state changed: "
            "InitialState -> InitializedState"
            ", dataAddress: "
            << this);
    }
    else
    {
        CORE_LOG_WARN(
            "CoreFramework internal initialization skipped: "
            "current state is not InitialState"
            ", dataAddress: "
            << this);
    }
}

void CoreFramework::DataPrivate::setStartupParameters(const std::vector<std::string>& args)
{
    std::scoped_lock<std::mutex> loc(mStartupParametersMutex);
    mStartupParameters = args;
    CORE_LOG_DEBUG(
        "Startup parameters updated, count: "
        << mStartupParameters.size()
        << ", dataAddress: "
        << this);
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
    if (!mDataPrivate->isInitializable())
    {
        CORE_LOG_WARN(
            "CoreFramework initialization skipped: "
            "not initializable in current state"
            ", address: "
            << this);
        return;
    }

    CORE_LOG_DEBUG(
        "CoreFramework initialization started, address: "
        << this);

    mDataPrivate->initialize();

    CORE_LOG_DEBUG(
        "CoreFramework initialization finished, address: "
        << this);
}

void CoreFramework::exitCoreFramework()
{
    if (!mDataPrivate->isRunnable())
    {
        CORE_LOG_WARN(
            "CoreFramework shutdown skipped: "
            "framework is not running"
            ", address: "
            << this);
        return;
    }

    CORE_LOG_DEBUG(
        "CoreFramework shutdown started, address: "
        << this);

    mDataPrivate->onExiting();

    CORE_LOG_DEBUG(
        "CoreFramework exit notification dispatch started"
        ", address: "
        << this);

    fireNotification(&ICoreFrameworkCallback::onCoreFrameworkExit);

    CORE_LOG_DEBUG(
        "CoreFramework exit notification dispatch finished"
        ", address: "
        << this);

    CORE_LOG_DEBUG(
        "CoreFramework service deinitialization started"
        ", address: "
        << this);

    deinitServices();

    CORE_LOG_DEBUG(
        "CoreFramework service deinitialization finished"
        ", address: "
        << this);

    CORE_LOG_DEBUG(
        "CoreFramework service unregistration started"
        ", address: "
        << this);

    unRegisterServices();

    CORE_LOG_DEBUG(
        "CoreFramework service unregistration finished"
        ", address: "
        << this);

    CORE_LOG_DEBUG(
        "CoreFramework shutdown finalization started"
        ", address: "
        << this);

    mDataPrivate->exitFinished();

    CORE_LOG_DEBUG(
        "CoreFramework shutdown finished, address: "
        << this);
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
        CORE_LOG_WARN(
            "Service initialization skipped: "
            "CoreFramework is not running"
            ", address: "
            << this);
        return;
    }

    if (!sortServicesByDependency())
    {
        CORE_LOG_ERROR(
            "Service initialization failed: "
            "unable to resolve dependency order"
            ", address: "
            << this);
        return;
    }

    const auto services = getAllServices();
    const auto registeredCount = services.size();
    std::size_t initializedCount = 0;

    CORE_LOG_DEBUG(
        "CoreFramework service initialization plan"
        ", registeredCount: "
        << registeredCount
        << ", address: "
        << this);

    for (const auto& weakService : services)
    {
        if (auto servicePtr = weakService.lock())
        {
            servicePtr->initComponent();
            ++initializedCount;
        }
    }

    CORE_LOG_DEBUG(
        "CoreFramework service initialization completed"
        ", initializedCount: "
        << initializedCount
        << ", registeredCount: "
        << registeredCount
        << ", address: "
        << this);

    fireNotification(&ICoreFrameworkCallback::onServiceInitialized);
}

void CoreFramework::deinitServices()
{
    // The storage is already sorted in initialization order, so shut services
    // down in its exact reverse: a service is torn down before its dependencies.
    auto services = getAllServices();
    std::size_t remainingCount = services.size();

    CORE_LOG_DEBUG(
        "CoreFramework service deinitialization plan"
        ", totalCount: "
        << remainingCount
        << ", address: "
        << this);

    for (auto iter = services.rbegin(); iter != services.rend(); ++iter)
    {
        if (auto servicePtr = iter->lock())
        {
            const auto serviceName = servicePtr->getServiceName();
            servicePtr->deinitComponent();
            --remainingCount;

            CORE_LOG_DEBUG(
                "CoreFramework service deinitialization progress"
                ", serviceName: "
                << serviceName
                << ", remainingCount: "
                << remainingCount
                << ", address: "
                << this);
        }
        else
        {
            --remainingCount;

            CORE_LOG_WARN(
                "CoreFramework service deinitialization skipped: "
                "service instance expired"
                ", remainingCount: "
                << remainingCount
                << ", address: "
                << this);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish CoreFramework Logic////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}
