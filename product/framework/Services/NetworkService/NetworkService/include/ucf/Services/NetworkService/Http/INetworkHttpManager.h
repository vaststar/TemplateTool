#pragma once

#include <functional>
#include <ucf/Services/ServiceCommonFile/ServiceExport.h>

namespace ucf::network::http{
class NetworkHttpRequest;
class NetworkHttpResponse;
using NetworkHttpResponseCallbackFunc = std::function<void(const NetworkHttpResponse& httpResponse)>;

class SERVICE_EXPORT INetworkHttpManager
{
public:
    virtual ~INetworkHttpManager() = default;
    virtual void sendHttpRequest(const NetworkHttpRequest& httpRequest, const NetworkHttpResponseCallbackFunc& callBackFunc) = 0;
};
}