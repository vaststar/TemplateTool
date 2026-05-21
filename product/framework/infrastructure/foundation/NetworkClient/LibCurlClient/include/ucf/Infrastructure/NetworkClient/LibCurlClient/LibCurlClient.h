#pragma once
#include <memory>
#include <ucf/Infrastructure/InfrastructureCommonFile/InfrastructureExport.h>
#include <ucf/Infrastructure/NetworkClient/NetworkModelTypes/Http/NetworkHttpTypes.h>
#include <ucf/Infrastructure/NetworkClient/NetworkModelTypes/Http/INetworkHttpClient.h>

namespace ucf::infrastructure::network::libcurl{
class Infrastructure_EXPORT LibCurlClient final: public ucf::infrastructure::network::http::INetworkHttpClient
{
public:
    LibCurlClient();
    ~LibCurlClient();
    LibCurlClient(const LibCurlClient&) = delete;
    LibCurlClient(LibCurlClient&&) = delete;
    LibCurlClient& operator=(const LibCurlClient&) = delete;
    LibCurlClient& operator=(LibCurlClient&&) = delete;

    virtual void makeGenericRequest(const ucf::infrastructure::network::http::NetworkHttpRequest& request,
                 ucf::infrastructure::network::http::HttpHeaderCallback headerCallback,
                 ucf::infrastructure::network::http::HttpBodyCallback bodyCallback,
                 ucf::infrastructure::network::http::HttpCompletionCallback completionCallback) override;
    virtual bool cancelRequest(const std::string& requestId) override;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}