#pragma once

#include <memory>
#include <functional>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpTypes.h>

#include <ucf/Services/NetworkService/Model/HttpTypes.h>
#include "NetworkCallbackHandler/INetworkHttpCallbackHandler.h"

namespace ucf::utilities::network::http{
class NetworkHttpResponse;
using NetworkHttpResponseCallbackFunc = std::function<void(const NetworkHttpResponse& httpResponse)>;
class NetworkHttpRequest;

}
namespace ucf::service::network::http{


class NetworkHttpRestCallbackHandler final: public INetworkHttpCallbackHandler
{
public:
    NetworkHttpRestCallbackHandler(const ucf::service::network::http::HttpRestRequest& restRequest, const HttpRestResponseCallbackFunc& restResponseCallback);
    virtual ~NetworkHttpRestCallbackHandler();
    NetworkHttpRestCallbackHandler(const NetworkHttpRestCallbackHandler&) = delete;
    NetworkHttpRestCallbackHandler(NetworkHttpRestCallbackHandler&&) = delete;
    NetworkHttpRestCallbackHandler& operator=(const NetworkHttpRestCallbackHandler&) = delete;
    NetworkHttpRestCallbackHandler& operator=(NetworkHttpRestCallbackHandler&&) = delete;
public:
    virtual const ucf::utilities::network::http::NetworkHttpRequest& getHttpRequest() const override;
    virtual void setResponseHeader(int statusCode, const ucf::utilities::network::http::NetworkHttpHeaders& headers, std::optional<ucf::utilities::network::http::ResponseErrorStruct> errorData) override;
    virtual void appendResponseBody(const ucf::utilities::network::http::ByteBuffer& buffer, bool isFinished) override;
    virtual void completeResponse(const ucf::utilities::network::http::HttpResponseMetrics& metrics) override;
    virtual bool shouldRetryRequest() const override;
    virtual const ucf::utilities::network::http::NetworkHttpRequest& prepareRetryRequest() override;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}