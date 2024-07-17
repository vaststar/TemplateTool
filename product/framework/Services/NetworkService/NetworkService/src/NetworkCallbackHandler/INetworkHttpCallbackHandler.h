#pragma once

#include <functional>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpTypes.h>

namespace ucf::utilities::network::http{
class NetworkHttpResponse;
using NetworkHttpResponseCallbackFunc = std::function<void(const NetworkHttpResponse& httpResponse)>;
class NetworkHttpRequest;
}

namespace ucf::service::network::http{
class INetworkHttpCallbackHandler
{
public:
    ~INetworkHttpCallbackHandler() = default;
    virtual const ucf::utilities::network::http::NetworkHttpRequest& getHttpRequest() const = 0;
    virtual void setResponseHeader(int statusCode, const ucf::utilities::network::http::NetworkHttpHeaders& headers, std::optional<ucf::utilities::network::http::ResponseErrorStruct> errorData) = 0;
    virtual void appendResponseBody(const ucf::utilities::network::http::ByteBuffer& buffer, bool isFinished) = 0;
    virtual void completeResponse(const ucf::utilities::network::http::HttpResponseMetrics& metrics) = 0;
    virtual bool shouldRetryRequest() const = 0;
    virtual const ucf::utilities::network::http::NetworkHttpRequest& prepareRetryRequest() = 0;
};
}