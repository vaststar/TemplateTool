#include <ucf/utilities/NetworkTransportUtils/runtime/NetworkRuntime.h>

#include "runtime/NetworkRuntimeAccess.h"
#include "runtime/RuntimeCore.h"

#include <utility>

namespace ucf::utilities::network {

NetworkRuntime::NetworkRuntime(NetworkRuntimeOptions options)
    : mCore{detail::RuntimeCore::create(options)}
{
}

NetworkRuntime::~NetworkRuntime()
{
    mCore->requestShutdown();
}

void NetworkRuntime::requestShutdown() noexcept
{
    mCore->requestShutdown();
}

void NetworkRuntime::shutdown()
{
    mCore->shutdown();
}

bool NetworkRuntime::isRunning() const noexcept
{
    return mCore->isRunning();
}

NetworkRuntimeState NetworkRuntime::state() const noexcept
{
    return mCore->state();
}

std::shared_ptr<detail::RuntimeCore> detail::NetworkRuntimeAccess::core(NetworkRuntime& runtime)
{
    return runtime.mCore;
}

asio::any_io_executor detail::NetworkRuntimeAccess::executor(NetworkRuntime& runtime)
{
    return runtime.mCore->executor();
}

bool detail::NetworkRuntimeAccess::post(
    NetworkRuntime& runtime,
    asio::any_io_executor executor,
    std::function<void()> task)
{
    return runtime.mCore->postUser(std::move(executor), std::move(task));
}

bool detail::NetworkRuntimeAccess::post(NetworkRuntime& runtime, std::function<void()> task)
{
    return runtime.mCore->postUser(runtime.mCore->executor(), std::move(task));
}

} // namespace ucf::utilities::network
