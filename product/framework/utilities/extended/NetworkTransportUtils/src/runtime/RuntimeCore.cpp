#include "runtime/RuntimeCore.h"

#include "NetworkTransportLogger.h"

#include <asio/post.hpp>

#include <exception>
#include <stdexcept>
#include <utility>

namespace ucf::utilities::network::detail {

RuntimeCore::ExecutionState::ExecutionState()
    : workGuard{asio::make_work_guard(ioContext)}
{
}

std::shared_ptr<RuntimeCore> RuntimeCore::create(const NetworkRuntimeOptions& options)
{
    if (options.ioThreadCount == 0)
    {
        throw std::invalid_argument{"NetworkRuntime ioThreadCount must be greater than zero"};
    }

    auto core = std::shared_ptr<RuntimeCore>{new RuntimeCore};
    core->mState = std::make_shared<ExecutionState>();
    core->startWorkers(options.ioThreadCount);
    NETWORK_TRANSPORT_LOG_INFO("NetworkRuntime", "Started with " << options.ioThreadCount << " I/O worker(s)");
    return core;
}

RuntimeCore::~RuntimeCore()
{
    requestShutdown();
    joinWorkers(true);
}

asio::any_io_executor RuntimeCore::executor() const
{
    return mState->ioContext.get_executor();
}

NetworkRuntimeState RuntimeCore::state() const noexcept
{
    return mState->publishedState.load(std::memory_order_acquire);
}

bool RuntimeCore::isRunning() const noexcept
{
    return state() == NetworkRuntimeState::Running;
}

bool RuntimeCore::runningInThisThread() const noexcept
{
    return mState->ioContext.get_executor().running_in_this_thread();
}

std::optional<RuntimeActivityId> RuntimeCore::startActivity(
    std::weak_ptr<RuntimeParticipant> participant,
    asio::any_io_executor targetExecutor,
    ActivityStarter starter)
{
    if (!starter || participant.expired())
    {
        return std::nullopt;
    }

    std::scoped_lock lock{mState->mutex};
    if (mState->publishedState.load(std::memory_order_relaxed) != NetworkRuntimeState::Running)
    {
        return std::nullopt;
    }

    RuntimeActivityId activityId = mState->nextActivityId++;
    if (activityId == 0)
    {
        activityId = mState->nextActivityId++;
    }

    const auto [iterator, inserted] = mState->participants.emplace(activityId, std::move(participant));
    if (!inserted)
    {
        return std::nullopt;
    }

    try
    {
        asio::post(std::move(targetExecutor), [starter = std::move(starter), activityId]() mutable {
            starter(activityId);
        });
    }
    catch (...)
    {
        mState->participants.erase(iterator);
        throw;
    }
    return activityId;
}

void RuntimeCore::finishActivity(RuntimeActivityId activityId) noexcept
{
    std::scoped_lock lock{mState->mutex};
    mState->participants.erase(activityId);
    releaseWorkIfQuiescedAndIdle(*mState);
}

bool RuntimeCore::postUser(asio::any_io_executor targetExecutor, std::function<void()> task)
{
    if (!task)
    {
        return false;
    }

    std::scoped_lock lock{mState->mutex};
    if (mState->publishedState.load(std::memory_order_relaxed) != NetworkRuntimeState::Running)
    {
        return false;
    }
    asio::post(std::move(targetExecutor), std::move(task));
    return true;
}

bool RuntimeCore::postInternal(asio::any_io_executor targetExecutor, std::function<void()> task) noexcept
{
    if (!task)
    {
        return false;
    }

    try
    {
        std::scoped_lock lock{mState->mutex};
        const NetworkRuntimeState currentState = mState->publishedState.load(std::memory_order_relaxed);
        if (currentState == NetworkRuntimeState::Stopped || currentState == NetworkRuntimeState::Faulted)
        {
            return false;
        }
        asio::post(std::move(targetExecutor), std::move(task));
        return true;
    }
    catch (const std::exception& exception)
    {
        NETWORK_TRANSPORT_LOG_FATAL("NetworkRuntime", "Internal actor post failed: " << exception.what());
    }
    catch (...)
    {
        NETWORK_TRANSPORT_LOG_FATAL("NetworkRuntime", "Internal actor post failed with an unknown exception");
    }
    // Failing an internal cleanup/event post can otherwise leave an activity
    // registered forever. Faulting is the only deterministic non-blocking exit.
    fault(mState);
    return false;
}

void RuntimeCore::requestShutdown() noexcept
{
    std::vector<std::shared_ptr<RuntimeParticipant>> participants;
    try
    {
        {
            std::scoped_lock lock{mState->mutex};
            if (mState->publishedState.load(std::memory_order_relaxed) != NetworkRuntimeState::Running)
            {
                return;
            }

            mState->publishedState.store(NetworkRuntimeState::Quiescing, std::memory_order_release);
            participants.reserve(mState->participants.size());
            for (auto iterator = mState->participants.begin(); iterator != mState->participants.end();)
            {
                if (auto participant = iterator->second.lock())
                {
                    participants.emplace_back(std::move(participant));
                    ++iterator;
                }
                else
                {
                    iterator = mState->participants.erase(iterator);
                }
            }
            releaseWorkIfQuiescedAndIdle(*mState);
        }

        NETWORK_TRANSPORT_LOG_INFO("NetworkRuntime", "Graceful shutdown requested for " << participants.size() << " active lifecycle(s)");
        for (const auto& participant : participants)
        {
            participant->requestStopFromRuntime();
        }
    }
    catch (const std::exception& exception)
    {
        NETWORK_TRANSPORT_LOG_FATAL("NetworkRuntime", "Could not initiate graceful shutdown: " << exception.what());
        fault(mState);
    }
    catch (...)
    {
        NETWORK_TRANSPORT_LOG_FATAL("NetworkRuntime", "Could not initiate graceful shutdown due to an unknown exception");
        fault(mState);
    }
}

void RuntimeCore::shutdown()
{
    if (runningInThisThread())
    {
        throw std::logic_error{"NetworkRuntime::shutdown() cannot run on an I/O thread"};
    }

    requestShutdown();
    {
        std::unique_lock lock{mState->mutex};
        mState->stoppedCondition.wait(lock, [state = mState] {
            return state->activeWorkers == 0;
        });
    }
    joinWorkers(false);
    NETWORK_TRANSPORT_LOG_INFO("NetworkRuntime", "Shutdown completed");
}

void RuntimeCore::startWorkers(std::size_t count)
{
    try
    {
        mWorkers.reserve(count);
        for (std::size_t index = 0; index < count; ++index)
        {
            std::thread worker{[state = mState]() noexcept {
                runWorker(state);
            }};
            {
                std::scoped_lock lock{mState->mutex};
                ++mState->activeWorkers;
            }
            mWorkers.emplace_back(std::move(worker));
        }
    }
    catch (...)
    {
        fault(mState);
        joinWorkers(false);
        throw;
    }
}

void RuntimeCore::joinWorkers(bool detachCurrent)
{
    std::scoped_lock lock{mJoinMutex};
    const bool destructorRunsOnWorker = detachCurrent && runningInThisThread();
    for (auto& worker : mWorkers)
    {
        if (!worker.joinable())
        {
            continue;
        }
        if (destructorRunsOnWorker)
        {
            // Joining a sibling here can deadlock if it is waiting for the
            // current handler to return. Every worker owns ExecutionState, so
            // detaching all workers is safe when RuntimeCore dies on one of
            // them.
            worker.detach();
        }
        else
        {
            worker.join();
        }
    }
}

void RuntimeCore::runWorker(const std::shared_ptr<ExecutionState>& state) noexcept
{
    try
    {
        state->ioContext.run();
    }
    catch (const std::exception& exception)
    {
        NETWORK_TRANSPORT_LOG_FATAL("NetworkRuntime", "Unhandled exception escaped an I/O handler: " << exception.what());
        fault(state);
    }
    catch (...)
    {
        NETWORK_TRANSPORT_LOG_FATAL("NetworkRuntime", "Unknown exception escaped an I/O handler");
        fault(state);
    }

    std::scoped_lock lock{state->mutex};
    if (state->activeWorkers > 0)
    {
        --state->activeWorkers;
    }
    if (state->activeWorkers == 0)
    {
        if (state->publishedState.load(std::memory_order_relaxed) != NetworkRuntimeState::Faulted)
        {
            state->publishedState.store(NetworkRuntimeState::Stopped, std::memory_order_release);
        }
        state->stoppedCondition.notify_all();
    }
}

void RuntimeCore::fault(const std::shared_ptr<ExecutionState>& state) noexcept
{
    try
    {
        std::scoped_lock lock{state->mutex};
        state->publishedState.store(NetworkRuntimeState::Faulted, std::memory_order_release);
        state->participants.clear();
        state->workGuard.reset();
        state->ioContext.stop();
    }
    catch (...)
    {
        state->publishedState.store(NetworkRuntimeState::Faulted, std::memory_order_release);
        state->ioContext.stop();
    }
}

void RuntimeCore::releaseWorkIfQuiescedAndIdle(ExecutionState& state) noexcept
{
    if (state.publishedState.load(std::memory_order_relaxed) == NetworkRuntimeState::Quiescing
        && state.participants.empty())
    {
        state.workGuard.reset();
    }
}

} // namespace ucf::utilities::network::detail
