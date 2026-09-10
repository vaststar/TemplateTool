#pragma once

#include <ucf/utilities/NetworkTransportUtils/runtime/NetworkRuntime.h>

#include <asio/any_io_executor.hpp>
#include <asio/executor_work_guard.hpp>
#include <asio/io_context.hpp>

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <unordered_map>
#include <vector>

namespace ucf::utilities::network::detail {

using RuntimeActivityId = std::uint64_t;

class RuntimeParticipant
{
public:
    virtual ~RuntimeParticipant() = default;
    virtual void requestStopFromRuntime() noexcept = 0;
};

/// Shared executor lease behind the public NetworkRuntime facade. Transports
/// keep this object alive, so destroying the facade cannot invalidate their
/// sockets while graceful shutdown is draining.
class RuntimeCore final : public std::enable_shared_from_this<RuntimeCore>
{
public:
    using ActivityStarter = std::function<void(RuntimeActivityId)>;

    static std::shared_ptr<RuntimeCore> create(const NetworkRuntimeOptions& options);
    ~RuntimeCore();

    RuntimeCore(const RuntimeCore&) = delete;
    RuntimeCore& operator=(const RuntimeCore&) = delete;

    [[nodiscard]] asio::any_io_executor executor() const;
    [[nodiscard]] NetworkRuntimeState state() const noexcept;
    [[nodiscard]] bool isRunning() const noexcept;
    [[nodiscard]] bool runningInThisThread() const noexcept;

    /// Atomically registers a lifecycle and schedules its first actor command.
    /// Shutdown cannot slip between those two operations.
    [[nodiscard]] std::optional<RuntimeActivityId> startActivity(
        std::weak_ptr<RuntimeParticipant> participant,
        asio::any_io_executor executor,
        ActivityStarter starter);

    void finishActivity(RuntimeActivityId activityId) noexcept;

    /// User work is accepted only while Running. Internal work remains accepted
    /// while Quiescing so transports can deliver terminal callbacks.
    [[nodiscard]] bool postUser(asio::any_io_executor executor, std::function<void()> task);
    [[nodiscard]] bool postInternal(asio::any_io_executor executor, std::function<void()> task) noexcept;

    void requestShutdown() noexcept;
    void shutdown();

private:
    using WorkGuard = asio::executor_work_guard<asio::io_context::executor_type>;

    struct ExecutionState final
    {
        ExecutionState();

        asio::io_context ioContext;
        std::optional<WorkGuard> workGuard;
        std::atomic<NetworkRuntimeState> publishedState{NetworkRuntimeState::Running};
        mutable std::mutex mutex;
        std::condition_variable stoppedCondition;
        std::unordered_map<RuntimeActivityId, std::weak_ptr<RuntimeParticipant>> participants;
        RuntimeActivityId nextActivityId{1};
        std::size_t activeWorkers{0};
    };

    RuntimeCore() = default;
    void startWorkers(std::size_t count);
    void joinWorkers(bool detachCurrent);
    static void runWorker(const std::shared_ptr<ExecutionState>& state) noexcept;
    static void fault(const std::shared_ptr<ExecutionState>& state) noexcept;
    static void releaseWorkIfQuiescedAndIdle(ExecutionState& state) noexcept;

private:
    std::shared_ptr<ExecutionState> mState;
    std::vector<std::thread> mWorkers;
    std::mutex mJoinMutex;
};

} // namespace ucf::utilities::network::detail
