#pragma once

#include <functional>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpTypes.h>

namespace ucf::utilities::network::http{
class NetworkHttpRequest;
}

namespace ucf::service::network::http{
class INetworkHttpHandler
{
public:
    ~INetworkHttpHandler() = default;
    virtual const ucf::utilities::network::http::NetworkHttpRequest& getHttpRequest() const = 0;
    virtual void setResponseHeader(int statusCode, const ucf::utilities::network::http::NetworkHttpHeaders& headers, std::optional<ucf::utilities::network::http::ResponseErrorStruct> errorData) = 0;
    virtual void appendResponseBody(const ucf::utilities::network::http::ByteBuffer& buffer, bool isFinished) = 0;
    virtual void completeResponse(const ucf::utilities::network::http::HttpResponseMetrics& metrics) = 0;
    
    virtual bool shouldRedirectRequest() const {return false;}
    virtual void prepareRedirectRequest() {}

    virtual bool shoudRetryRequest() const {return false;}
    virtual int getRetryAfterMillSecs() const {return 1;}
    virtual void prepareRetryRequest() {}


};
}