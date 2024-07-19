#pragma once

#include <source_location>
#include <functional>
#include <ucf/Services/ServiceCommonFile/ServiceExport.h>
namespace ucf::service::network::http{
class HttpRestRequest;
class HttpRestResponse;
using HttpRestResponseCallbackFunc = std::function<void(const ucf::service::network::http::HttpRestResponse& restResponse)>;

class SERVICE_EXPORT INetworkHttpManager
{
public:
    virtual ~INetworkHttpManager() = default;

    
    virtual void sendHttpRestRequest(const ucf::service::network::http::HttpRestRequest& restRequest, const ucf::service::network::http::HttpRestResponseCallbackFunc& restResponseCallback, const std::source_location location = std::source_location::current()) = 0;
};
}