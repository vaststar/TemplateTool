#pragma once

#include <ucf/utilities/NetworkTransportUtils/NetworkTransportUtilsExport.h>

#include <cstddef>
#include <memory>

namespace ucf::utilities::network {

namespace detail {
class NetworkRuntimeAccess;
class RuntimeCore;
}

enum class NetworkRuntimeState
{
    Running,
    Quiescing,
    Stopped,
    Faulted
};

struct NetworkRuntimeOptions final
{
    // One thread is sufficient for the common case. Increase this only when a
    // shared runtime has enough concurrent I/O work to benefit from it.
    std::size_t ioThreadCount{1};
};

/// Owns the process-independent execution context used by network transports.
///
/// Construction starts the I/O threads. A runtime is one-shot: after shutdown
/// begins it cannot be restarted. Transports retain a private executor lease,
/// so destroying this facade first requests graceful shutdown instead of
/// invalidating live sockets.
class NETWORK_TRANSPORT_UTILS_API NetworkRuntime final
{
public:
    /// Starts the configured number of I/O threads.
    /// Throws std::invalid_argument when ioThreadCount is zero and propagates
    /// thread-creation failures from the standard library.
    explicit NetworkRuntime(NetworkRuntimeOptions options = {});
    ~NetworkRuntime();

    NetworkRuntime(const NetworkRuntime&) = delete;
    NetworkRuntime& operator=(const NetworkRuntime&) = delete;
    NetworkRuntime(NetworkRuntime&&) = delete;
    NetworkRuntime& operator=(NetworkRuntime&&) = delete;

    /// Starts graceful shutdown without waiting. New lifecycle and send
    /// commands are rejected, while registered transports retain executor
    /// access long enough to close and drain their terminal callbacks.
    void requestShutdown() noexcept;

    /// Gracefully closes registered transports, drains internal cleanup work,
    /// and waits for every I/O thread to exit.
    ///
    /// This operation is thread-safe and idempotent. On return, the runtime is
    /// fully stopped. Calling it from one of this runtime's own I/O threads
    /// throws std::logic_error. Unexpected exceptions escaping an internal I/O
    /// handler are contained and stop the runtime; they are never delayed and
    /// rethrown by shutdown().
    void shutdown();

    /// True only while the runtime accepts new user work.
    [[nodiscard]] bool isRunning() const noexcept;
    [[nodiscard]] NetworkRuntimeState state() const noexcept;

private:
    std::shared_ptr<detail::RuntimeCore> mCore;

    friend class detail::NetworkRuntimeAccess;
};

} // namespace ucf::utilities::network
