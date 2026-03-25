#pragma once
#include <memory>
#include <ucf/Agents/AgentsCommonFile/AgentsExport.h>
#include <ucf/Agents/NetworkAgent/NetworkModelTypes/Http/NetworkHttpTypes.h>
#include <ucf/Agents/NetworkAgent/NetworkModelTypes/Http/INetworkHttpClient.h>

namespace ucf::agents::network::libcurl{
class Agents_EXPORT LibCurlClient final: public ucf::agents::network::http::INetworkHttpClient
{
public:
    LibCurlClient();
    ~LibCurlClient();
    LibCurlClient(const LibCurlClient&) = delete;
    LibCurlClient(LibCurlClient&&) = delete;
    LibCurlClient& operator=(const LibCurlClient&) = delete;
    LibCurlClient& operator=(LibCurlClient&&) = delete;

    virtual void makeGenericRequest(const ucf::agents::network::http::NetworkHttpRequest& request,
                 ucf::agents::network::http::HttpHeaderCallback headerCallback,
                 ucf::agents::network::http::HttpBodyCallback bodyCallback,
                 ucf::agents::network::http::HttpCompletionCallback completionCallback) override;
    virtual bool cancelRequest(const std::string& requestId) override;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}