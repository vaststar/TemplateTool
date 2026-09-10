#include <catch2/catch_test_macros.hpp>

#include <ucf/utilities/NetworkTransportUtils/runtime/NetworkRuntime.h>

#include "runtime/NetworkRuntimeAccess.h"

#include <asio/strand.hpp>

#include <chrono>
#include <condition_variable>
#include <future>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>

namespace {

using namespace std::chrono_literals;
using ucf::utilities::network::NetworkRuntime;
using ucf::utilities::network::NetworkRuntimeOptions;
using ucf::utilities::network::detail::NetworkRuntimeAccess;

} // namespace

TEST_CASE("NetworkRuntime executes work without exposing Asio", "[NetworkTransportUtils][NetworkRuntime]")
{
    NetworkRuntime runtime;
    REQUIRE(runtime.isRunning());

    auto completion = std::make_shared<std::promise<bool>>();
    auto result = completion->get_future();
    const auto callerThread = std::this_thread::get_id();

    REQUIRE(NetworkRuntimeAccess::post(runtime, [callerThread, completion] {
        completion->set_value(std::this_thread::get_id() != callerThread);
    }));
    REQUIRE(result.wait_for(2s) == std::future_status::ready);
    REQUIRE(result.get());

    REQUIRE_NOTHROW(runtime.shutdown());
    REQUIRE_FALSE(runtime.isRunning());
    REQUIRE_FALSE(NetworkRuntimeAccess::post(runtime, [] {}));
    REQUIRE_NOTHROW(runtime.shutdown());
}

TEST_CASE("NetworkRuntime rejects strand work after shutdown", "[NetworkTransportUtils][NetworkRuntime]")
{
    NetworkRuntime runtime;
    const auto strand = asio::make_strand(NetworkRuntimeAccess::executor(runtime));

    runtime.shutdown();

    REQUIRE_FALSE(NetworkRuntimeAccess::post(runtime, strand, [] {}));
}

TEST_CASE("NetworkRuntime supports an explicit I/O thread count", "[NetworkTransportUtils][NetworkRuntime]")
{
    struct Gate final
    {
        std::mutex mutex;
        std::condition_variable condition;
        std::size_t started{0};
        bool released{false};
    };

    NetworkRuntimeOptions options;
    options.ioThreadCount = 2;

    NetworkRuntime runtime{options};
    auto gate = std::make_shared<Gate>();

    const auto blockingTask = [gate] {
        std::unique_lock lock{gate->mutex};
        ++gate->started;
        gate->condition.notify_all();
        gate->condition.wait(lock, [gate] {
            return gate->released;
        });
    };

    const bool firstAccepted = NetworkRuntimeAccess::post(runtime, blockingTask);
    const bool secondAccepted = NetworkRuntimeAccess::post(runtime, blockingTask);

    bool ranConcurrently = false;
    {
        std::unique_lock lock{gate->mutex};
        ranConcurrently = gate->condition.wait_for(lock, 2s, [gate] {
            return gate->started == 2;
        });
        gate->released = true;
    }
    gate->condition.notify_all();

    REQUIRE(firstAccepted);
    REQUIRE(secondAccepted);
    REQUIRE(ranConcurrently);
    runtime.shutdown();
}

TEST_CASE("NetworkRuntime rejects an empty worker set", "[NetworkTransportUtils][NetworkRuntime]")
{
    NetworkRuntimeOptions options;
    options.ioThreadCount = 0;

    REQUIRE_THROWS_AS(NetworkRuntime{options}, std::invalid_argument);
}

TEST_CASE("NetworkRuntime stops instead of propagating I/O work exceptions", "[NetworkTransportUtils][NetworkRuntime]")
{
    NetworkRuntime runtime;
    auto entered = std::make_shared<std::promise<void>>();
    auto enteredFuture = entered->get_future();

    REQUIRE(NetworkRuntimeAccess::post(runtime, [entered] {
        entered->set_value();
        throw std::runtime_error{"test worker failure"};
    }));
    REQUIRE(enteredFuture.wait_for(2s) == std::future_status::ready);

    const auto stopDeadline = std::chrono::steady_clock::now() + 2s;
    while (runtime.isRunning() && std::chrono::steady_clock::now() < stopDeadline)
    {
        std::this_thread::yield();
    }

    REQUIRE_FALSE(runtime.isRunning());
    REQUIRE_FALSE(NetworkRuntimeAccess::post(runtime, [] {}));
    REQUIRE_NOTHROW(runtime.shutdown());
}

TEST_CASE("NetworkRuntime rejects synchronous shutdown on an I/O thread", "[NetworkTransportUtils][NetworkRuntime]")
{
    NetworkRuntime runtime;
    auto completion = std::make_shared<std::promise<bool>>();
    auto rejected = completion->get_future();

    REQUIRE(NetworkRuntimeAccess::post(runtime, [&runtime, completion] {
        try
        {
            runtime.shutdown();
            completion->set_value(false);
        }
        catch (const std::logic_error&)
        {
            completion->set_value(true);
        }
    }));

    REQUIRE(rejected.wait_for(2s) == std::future_status::ready);
    REQUIRE(rejected.get());
    REQUIRE(runtime.isRunning());
    REQUIRE_NOTHROW(runtime.shutdown());
}

TEST_CASE("NetworkRuntime can release its final owner on an I/O thread", "[NetworkTransportUtils][NetworkRuntime]")
{
    auto runtime = std::make_shared<NetworkRuntime>();
    std::weak_ptr<NetworkRuntime> weakRuntime = runtime;

    auto completion = std::make_shared<std::promise<void>>();
    auto completed = completion->get_future();
    auto workerOwner = runtime;

    REQUIRE(NetworkRuntimeAccess::post(*runtime, [workerOwner = std::move(workerOwner), completion]() mutable {
        workerOwner.reset();
        completion->set_value();
    }));

    runtime.reset();
    REQUIRE(completed.wait_for(2s) == std::future_status::ready);
    REQUIRE(weakRuntime.expired());
}

TEST_CASE("NetworkRuntime destruction on a worker does not join a blocked sibling", "[NetworkTransportUtils][NetworkRuntime]")
{
    NetworkRuntimeOptions options;
    options.ioThreadCount = 2;
    auto runtime = std::make_shared<NetworkRuntime>(options);

    auto blockerEntered = std::make_shared<std::promise<void>>();
    auto blockerEnteredFuture = blockerEntered->get_future();
    auto releaseBlocker = std::make_shared<std::promise<void>>();
    auto releaseFuture = releaseBlocker->get_future().share();
    REQUIRE(NetworkRuntimeAccess::post(*runtime, [blockerEntered, releaseFuture] {
        blockerEntered->set_value();
        releaseFuture.wait();
    }));
    REQUIRE(blockerEnteredFuture.wait_for(2s) == std::future_status::ready);

    auto destroyed = std::make_shared<std::promise<void>>();
    auto destroyedFuture = destroyed->get_future();
    auto finalWorkerOwner = runtime;
    REQUIRE(NetworkRuntimeAccess::post(*runtime, [finalWorkerOwner = std::move(finalWorkerOwner), destroyed]() mutable {
        finalWorkerOwner.reset();
        destroyed->set_value();
    }));
    runtime.reset();

    const bool destructorReturnedBeforeSibling = destroyedFuture.wait_for(500ms) == std::future_status::ready;
    releaseBlocker->set_value();
    REQUIRE(destructorReturnedBeforeSibling);
}
