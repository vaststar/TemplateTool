#pragma once
#include <memory>
#include <ucf/Agents/NetworkAgent/NetworkModelTypes/Http/NetworkHttpTypes.h>
#include <ucf/Agents/NetworkAgent/NetworkModelTypes/Http/NetworkHttpRequest.h>
#include <ucf/Agents/AgentsCommonFile/AgentsExport.h>

namespace ucf::agents::network::http{

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

    virtual void makeGenericRequest(const ucf::agents::network::http::NetworkHttpRequest& request,
        ucf::agents::network::http::HttpHeaderCallback headerCallback,
        ucf::agents::network::http::HttpBodyCallback bodyCallback,
        ucf::agents::network::http::HttpCompletionCallback completionCallback) = 0;
    virtual bool cancelRequest(const std::string& requestId) = 0;
};
}