#pragma once

#include <memory>
#include <functional>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpTypes.h>

#include "NetworkHttpHandler/INetworkHttpHandler.h"

namespace ucf::utilities::network::http{
class NetworkHttpResponse;
using NetworkHttpResponseCallbackFunc = std::function<void(const NetworkHttpResponse& httpResponse)>;
class NetworkHttpRequest;

}
namespace ucf::service::network::http{
class HttpRawRequest;


class NetworkHttpRawHandler final: public INetworkHttpHandler
{
public:
    NetworkHttpRawHandler(const ucf::service::network::http::HttpRawRequest& restRequest, const HttpRawResponseCallbackFunc& restResponseCallback);
    virtual ~NetworkHttpRawHandler();
    NetworkHttpRawHandler(const NetworkHttpRawHandler&) = delete;
    NetworkHttpRawHandler(NetworkHttpRawHandler&&) = delete;
    NetworkHttpRawHandler& operator=(const NetworkHttpRawHandler&) = delete;
    NetworkHttpRawHandler& operator=(NetworkHttpRawHandler&&) = delete;
public:
    virtual const ucf::utilities::network::http::NetworkHttpRequest& getHttpRequest() const override;
    virtual void setResponseHeader(int statusCode, const ucf::utilities::network::http::NetworkHttpHeaders& headers, std::optional<ucf::utilities::network::http::ResponseErrorStruct> errorData) override;
    virtual void appendResponseBody(const ucf::utilities::network::http::ByteBuffer& buffer, bool isFinished) override;
    virtual void completeResponse(const ucf::utilities::network::http::HttpResponseMetrics& metrics) override;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}