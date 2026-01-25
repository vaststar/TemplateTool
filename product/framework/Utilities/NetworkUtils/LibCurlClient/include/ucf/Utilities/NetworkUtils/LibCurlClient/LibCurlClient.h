#pragma once
#include <memory>
#include <ucf/Utilities/NetworkUtils/LibCurlClient/LibCurlClientExport.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpTypes.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/INetworkHttpClient.h>

namespace ucf::utilities::network::libcurl{
class LIBCURLCLIENT_EXPORT LibCurlClient final: public ucf::utilities::network::http::INetworkHttpClient
{
public:
    LibCurlClient();
    ~LibCurlClient();
    LibCurlClient(const LibCurlClient&) = delete;
    LibCurlClient(LibCurlClient&&) = delete;
    LibCurlClient& operator=(const LibCurlClient&) = delete;
    LibCurlClient& operator=(LibCurlClient&&) = delete;

    virtual void makeGenericRequest(const ucf::utilities::network::http::NetworkHttpRequest& request,
                 ucf::utilities::network::http::HttpHeaderCallback headerCallback,
                 ucf::utilities::network::http::HttpBodyCallback bodyCallback,
                 ucf::utilities::network::http::HttpCompletionCallback completionCallback) override;
    virtual bool cancelRequest(const std::string& requestId) override;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}