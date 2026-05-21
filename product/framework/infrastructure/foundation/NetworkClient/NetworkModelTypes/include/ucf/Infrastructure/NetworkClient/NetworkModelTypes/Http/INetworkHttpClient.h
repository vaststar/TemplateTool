#pragma once
#include <memory>
#include <ucf/Infrastructure/NetworkClient/NetworkModelTypes/Http/NetworkHttpTypes.h>
#include <ucf/Infrastructure/NetworkClient/NetworkModelTypes/Http/NetworkHttpRequest.h>
#include <ucf/Infrastructure/InfrastructureCommonFile/InfrastructureExport.h>

namespace ucf::infrastructure::network::http{

/**
 * @brief Abstract interface for HTTP client implementations.
 * 
 * Provides a streaming callback-based API for making HTTP requests.
 * Implementations should handle connection pooling, SSL, and async I/O.
 */
class INetworkHttpClient
{
public:
    INetworkHttpClient() = default;
    INetworkHttpClient(const INetworkHttpClient&) = delete;
    INetworkHttpClient(INetworkHttpClient&&) = delete;
    INetworkHttpClient& operator=(const INetworkHttpClient&) = delete;
    INetworkHttpClient& operator=(INetworkHttpClient&&) = delete;
    virtual ~INetworkHttpClient() = default;

    virtual void makeGenericRequest(const ucf::infrastructure::network::http::NetworkHttpRequest& request,
        ucf::infrastructure::network::http::HttpHeaderCallback headerCallback,
        ucf::infrastructure::network::http::HttpBodyCallback bodyCallback,
        ucf::infrastructure::network::http::HttpCompletionCallback completionCallback) = 0;
    virtual bool cancelRequest(const std::string& requestId) = 0;
};
}