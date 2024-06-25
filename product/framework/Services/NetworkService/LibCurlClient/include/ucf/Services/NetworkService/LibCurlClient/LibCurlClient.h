#pragma once
#include <memory>
#include <functional>
#include <ucf/Services/NetworkService/LibCurlClient/LibCurlClientExport.h>
#include <ucf/Services/NetworkService/NetworkModelTypes/Http/NetworkHttpTypes.h>

namespace ucf::service::network::libcurl{
class LIBCURLCLIENT_EXPORT LibCurlClient final
{
public:
    LibCurlClient();
    ~LibCurlClient();
    LibCurlClient(const LibCurlClient&) = delete;
    LibCurlClient(LibCurlClient&&) = delete;
    LibCurlClient& operator=(const LibCurlClient&) = delete;
    LibCurlClient& operator=(LibCurlClient&&) = delete;

    void makeGenericRequest(const ucf::service::network::http::NetworkHttpRequest& request, const ucf::service::network::http::HttpHeaderCallback& headerCallback, const ucf::service::network::http::HttpBodyCallback& bodyCallback, const ucf::service::network::http::HttpCompletionCallback& completionCallback);
    void startService();
    void stopService();
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}