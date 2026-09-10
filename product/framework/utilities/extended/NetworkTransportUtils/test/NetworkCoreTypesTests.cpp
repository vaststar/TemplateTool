#include <catch2/catch_test_macros.hpp>

#include <ucf/utilities/NetworkTransportUtils/NetworkTransportUtils.h>

#include "core/ExceptionErrorMapper.h"
#include "core/SteadyClockUtils.h"
#include "tcp/TcpCloseInfoUtils.h"

#include <asio/error.hpp>

#include <chrono>
#include <memory>
#include <string>
#include <system_error>
#include <utility>

namespace {

using ucf::utilities::network::Endpoint;
using ucf::utilities::network::CloseOrigin;
using ucf::utilities::network::NetworkError;
using ucf::utilities::network::NetworkErrorCode;
using ucf::utilities::network::NetworkOperation;
using ucf::utilities::network::NetworkResult;
using ucf::utilities::network::TransportCloseInfo;

} // namespace

TEST_CASE("Endpoint represents host names and numeric addresses", "[NetworkTransportUtils][Core]")
{
    const Endpoint hostName{"example.test", 443};
    const Endpoint sameHostName{"example.test", 443};
    const Endpoint ipv6{"::1", 8080};

    REQUIRE(hostName == sameHostName);
    REQUIRE_FALSE(hostName == ipv6);
    REQUIRE(ipv6.host == "::1");
    REQUIRE(ipv6.port == 8080);
}

TEST_CASE("Endpoint preserves port zero for dynamic binding", "[NetworkTransportUtils][Core]")
{
    const Endpoint endpoint{"", 0};

    REQUIRE(endpoint.host.empty());
    REQUIRE(endpoint.port == 0);
}

TEST_CASE("NetworkError separates stable and native error information", "[NetworkTransportUtils][Core]")
{
    const std::error_code nativeError = std::make_error_code(std::errc::connection_refused);
    const NetworkError error{
        NetworkErrorCode::ConnectionRefused,
        NetworkOperation::Connect,
        nativeError,
        "The remote endpoint refused the connection"};

    REQUIRE(error.code == NetworkErrorCode::ConnectionRefused);
    REQUIRE(error.operation == NetworkOperation::Connect);
    REQUIRE(error.nativeError == nativeError);
    REQUIRE_FALSE(error.diagnostic.empty());
}

TEST_CASE("NetworkResult carries move-only success values", "[NetworkTransportUtils][Core]")
{
    auto result = NetworkResult<std::unique_ptr<int>>::success(std::make_unique<int>(42));

    REQUIRE(result.hasValue());
    REQUIRE(static_cast<bool>(result));
    REQUIRE(*result.value() == 42);

    std::unique_ptr<int> value = std::move(result).value();
    REQUIRE(*value == 42);
}

TEST_CASE("NetworkResult represents value-less success and structured failure", "[NetworkTransportUtils][Core]")
{
    const auto success = NetworkResult<void>::success();
    const auto failure = NetworkResult<void>::failure(NetworkError{
        NetworkErrorCode::InvalidState,
        NetworkOperation::Connect,
        {},
        "The command cannot run in the current state"});

    REQUIRE(success.hasValue());
    REQUIRE_FALSE(failure.hasValue());
    REQUIRE(failure.error().code == NetworkErrorCode::InvalidState);
    REQUIRE(failure.error().operation == NetworkOperation::Connect);
}

TEST_CASE("TransportCloseInfo separates origin cause and discarded capacity", "[NetworkTransportUtils][Core]")
{
    const TransportCloseInfo info{
        CloseOrigin::Error,
        NetworkError{NetworkErrorCode::ConnectionReset, NetworkOperation::Read, {}, "peer reset"},
        4096};

    REQUIRE(info.origin == CloseOrigin::Error);
    REQUIRE(info.cause.has_value());
    REQUIRE(info.cause->code == NetworkErrorCode::ConnectionReset);
    REQUIRE(info.discardedWriteBytes == 4096);
}

TEST_CASE("TCP close classification is consistent for read and write failures", "[NetworkTransportUtils][Core]")
{
    const auto readClose = ucf::utilities::network::detail::makeTcpCloseInfo(
        NetworkError{NetworkErrorCode::EndOfStream, NetworkOperation::Read, {}, "peer EOF"});
    const auto writeClose = ucf::utilities::network::detail::makeTcpCloseInfo(
        NetworkError{NetworkErrorCode::ConnectionReset, NetworkOperation::Write, {}, "peer reset"});
    const auto connectClose = ucf::utilities::network::detail::makeTcpCloseInfo(
        NetworkError{NetworkErrorCode::ConnectionReset, NetworkOperation::Connect, {}, "connect failed"});

    REQUIRE(readClose.origin == CloseOrigin::Peer);
    REQUIRE(writeClose.origin == CloseOrigin::Peer);
    REQUIRE(connectClose.origin == CloseOrigin::Error);
}

TEST_CASE("Synchronous system exceptions use the stable network error mapping", "[NetworkTransportUtils][Core]")
{
    const std::system_error exception{asio::error::make_error_code(asio::error::connection_refused)};
    const auto error = ucf::utilities::network::makeExceptionError(exception, NetworkOperation::Connect);

    REQUIRE(error.code == NetworkErrorCode::ConnectionRefused);
    REQUIRE(error.operation == NetworkOperation::Connect);
    REQUIRE(error.nativeError == exception.code());
}

TEST_CASE("Steady deadlines saturate instead of overflowing", "[NetworkTransportUtils][Core]")
{
    const auto deadline = ucf::utilities::network::detail::steadyDeadlineAfter(
        std::chrono::milliseconds::max());

    REQUIRE(deadline == std::chrono::steady_clock::time_point::max());
}
