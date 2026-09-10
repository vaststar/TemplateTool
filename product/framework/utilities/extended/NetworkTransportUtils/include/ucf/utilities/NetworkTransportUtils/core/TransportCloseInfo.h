#pragma once

#include <ucf/utilities/NetworkTransportUtils/core/NetworkError.h>

#include <cstddef>
#include <optional>

namespace ucf::utilities::network {

/// Identifies who or what ended an accepted transport lifecycle.
enum class CloseOrigin
{
    Local,
    Peer,
    RuntimeShutdown,
    Error
};

/// Describes the single terminal event emitted for a transport lifecycle.
///
/// A lifecycle starts when connect(), start(), or bind() accepts a command and,
/// while its transport object remains alive, ends with exactly one corresponding
/// close/stopped callback. Destruction deliberately suppresses callbacks. cause
/// is set for peer and error closes when diagnostic information is available.
struct TransportCloseInfo final
{
    CloseOrigin origin{CloseOrigin::Local};
    std::optional<NetworkError> cause;
    // Bytes accepted by send()/sendTo() but not confirmed written to the
    // operating system before the lifecycle ended.
    std::size_t discardedWriteBytes{0};
};

} // namespace ucf::utilities::network
