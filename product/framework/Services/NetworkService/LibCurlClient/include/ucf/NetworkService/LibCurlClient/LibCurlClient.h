#pragma once
#include <memory>
#include <functional>
#include <ucf/NetworkService/LibCurlClient/LibCurlClientExport.h>

namespace ucf::network::http{
class NetworkHttpRequest;
class NetworkHttpResponse;
using NetworkHttpResponseCallbackFunc = std::function<void(const NetworkHttpResponse& httpResponse)>;
}
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

    void makeHttpRequest(const ucf::network::http::NetworkHttpRequest& httpRequest, const ucf::network::http::NetworkHttpResponseCallbackFunc& callBackFunc);
    void startService();
    void stopService();
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}