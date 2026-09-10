#pragma once

#include <ucf/utilities/NetworkTransportUtils/runtime/NetworkRuntime.h>

#include <asio/any_io_executor.hpp>

#include <functional>
#include <memory>

namespace ucf::utilities::network::detail {

// Source-only bridge for transport implementations. Keeping this header under
// src prevents Asio types from leaking into the installed public API.
class NetworkRuntimeAccess final
{
public:
    [[nodiscard]] static std::shared_ptr<RuntimeCore> core(NetworkRuntime& runtime);
    [[nodiscard]] static NETWORK_TRANSPORT_UTILS_API asio::any_io_executor executor(NetworkRuntime& runtime);
    [[nodiscard]] static NETWORK_TRANSPORT_UTILS_API bool post(
        NetworkRuntime& runtime,
        asio::any_io_executor executor,
        std::function<void()> task);
    [[nodiscard]] static NETWORK_TRANSPORT_UTILS_API bool post(NetworkRuntime& runtime, std::function<void()> task);
};

} // namespace ucf::utilities::network::detail
