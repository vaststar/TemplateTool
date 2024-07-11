#pragma once

#include <source_location>
#include <functional>
#include <ucf/Services/ServiceCommonFile/ServiceExport.h>

namespace ucf::service::network::http{
class NetworkHttpRequest;
class NetworkHttpResponse;
using NetworkHttpResponseCallbackFunc = std::function<void(const NetworkHttpResponse& httpResponse)>;

class SERVICE_EXPORT INetworkHttpManager
{
public:
    virtual ~INetworkHttpManager() = default;

    virtual void sendHttpRequest(const NetworkHttpRequest& httpRequest, const NetworkHttpResponseCallbackFunc& callBackFunc, const std::source_location location = std::source_location::current()) = 0;
    
    // virtual void HttpRequest
    // virtual void downloadContentToMemory();
};
}