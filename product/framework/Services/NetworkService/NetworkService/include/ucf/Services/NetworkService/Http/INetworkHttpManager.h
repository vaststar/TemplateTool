#pragma once

#include <source_location>
#include <functional>
#include <ucf/Services/ServiceCommonFile/ServiceExport.h>

namespace ucf::utilities::network::http{
class NetworkHttpRequest;
class NetworkHttpResponse;
using NetworkHttpResponseCallbackFunc = std::function<void(const NetworkHttpResponse& httpResponse)>;

}
namespace ucf::service::network::http{

class SERVICE_EXPORT INetworkHttpManager
{
public:
    virtual ~INetworkHttpManager() = default;

    virtual 
    virtual void sendHttpRequest(const ucf::utilities::network::http::NetworkHttpRequest& httpRequest, const ucf::utilities::network::http::NetworkHttpResponseCallbackFunc& callBackFunc, const std::source_location location = std::source_location::current()) = 0;
    
    virtual void HttprRestRequest();
    // virtual void downloadContentToMemory();
};
}