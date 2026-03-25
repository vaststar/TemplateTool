#pragma once

#include <functional>
#include <ucf/Agents/NetworkAgent/NetworkModelTypes/Http/NetworkHttpTypes.h>

namespace ucf::agents::network::http{
class NetworkHttpRequest;
}

namespace ucf::service::network::http{
class INetworkHttpHandler
{
public:
    INetworkHttpHandler() = default;
    INetworkHttpHandler(const INetworkHttpHandler&) = delete;
    INetworkHttpHandler(INetworkHttpHandler&&) = delete;
    INetworkHttpHandler& operator=(const INetworkHttpHandler&) = delete;
    INetworkHttpHandler& operator=(INetworkHttpHandler&&) = delete;
    virtual ~INetworkHttpHandler() = default;
public:
    virtual const ucf::agents::network::http::NetworkHttpRequest& getHttpRequest() const = 0;
    virtual void setResponseHeader(int statusCode, const ucf::agents::network::http::NetworkHttpHeaders& headers, std::optional<ucf::agents::network::http::ResponseErrorStruct> errorData) = 0;
    virtual void appendResponseBody(const ucf::agents::network::http::ByteBuffer& buffer, bool isFinished) = 0;
    virtual void completeResponse(const ucf::agents::network::http::HttpResponseMetrics& metrics) = 0;
    
    virtual bool shouldRedirectRequest() const {return false;}
    virtual void prepareRedirectRequest() {}

    virtual bool shouldRetryRequest() const {return false;}
    virtual int getRetryAfterMillSecs() const {return 1;}
    virtual void prepareRetryRequest() {}


};
}