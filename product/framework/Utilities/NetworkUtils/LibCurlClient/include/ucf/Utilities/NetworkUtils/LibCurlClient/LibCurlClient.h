#pragma once
#include <memory>
#include <ucf/Utilities/NetworkUtils/LibCurlClient/LibCurlClientExport.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpTypes.h>

namespace ucf::utilities::network::http{
    class NetworkHttpRequest;
}
namespace ucf::utilities::network::libcurl{
class LIBCURLCLIENT_EXPORT LibCurlClient final
{
public:
    LibCurlClient();
    ~LibCurlClient();
    LibCurlClient(const LibCurlClient&) = delete;
    LibCurlClient(LibCurlClient&&) = delete;
    LibCurlClient& operator=(const LibCurlClient&) = delete;
    LibCurlClient& operator=(LibCurlClient&&) = delete;

    void makeGenericRequest(const ucf::utilities::network::http::NetworkHttpRequest& request, const ucf::utilities::network::http::HttpHeaderCallback& headerCallback, const ucf::utilities::network::http::HttpBodyCallback& bodyCallback, const ucf::utilities::network::http::HttpCompletionCallback& completionCallback);
    void startService();
    void stopService();
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}