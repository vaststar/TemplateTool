#pragma once
#include <memory>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpTypes.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpRequest.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/NetworkModelTypesExport.h>

namespace ucf::utilities::network::http{

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

    virtual void makeGenericRequest(const ucf::utilities::network::http::NetworkHttpRequest& request,
        ucf::utilities::network::http::HttpHeaderCallback headerCallback,
        ucf::utilities::network::http::HttpBodyCallback bodyCallback,
        ucf::utilities::network::http::HttpCompletionCallback completionCallback) = 0;
    virtual bool cancelRequest(const std::string& requestId) = 0;
};
}