#pragma once
#include <memory>
#include <functional>
#include <ucf/NetworkService/LibCurlClient/LibCurlClientExport.h>
#include <ucf/NetworkService/NetworkModelTypes/Http/NetworkHttpTypes.h>

namespace ucf::network::libcurl{
class LIBCURLCLIENT_EXPORT LibCurlClient final
{
public:
    LibCurlClient();
    ~LibCurlClient();
    LibCurlClient(const LibCurlClient&) = delete;
    LibCurlClient(LibCurlClient&&) = delete;
    LibCurlClient& operator=(const LibCurlClient&) = delete;
    LibCurlClient& operator=(LibCurlClient&&) = delete;

    void makeGenericRequest(const ucf::network::http::NetworkHttpRequest& request, const ucf::network::http::HttpHeaderCallback& headerCallback, const ucf::network::http::HttpBodyCallback& bodyCallback, const ucf::network::http::HttpCompletionCallback& completionCallback);
    void startService();
    void stopService();
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}