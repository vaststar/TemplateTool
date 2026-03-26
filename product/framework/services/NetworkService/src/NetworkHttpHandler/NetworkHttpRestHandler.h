#pragma once

#include <memory>
#include <functional>
#include <ucf/Agents/NetworkAgent/NetworkModelTypes/Http/NetworkHttpTypes.h>
#include <ucf/Services/NetworkService/Model/HttpDeclareTypes.h>

#include "NetworkHttpHandler/INetworkHttpHandler.h"

namespace ucf::agents::network::http{
class NetworkHttpRequest;
}
namespace ucf::service::network::http{
class HttpRestRequest;

class NetworkHttpRestHandler final: public INetworkHttpHandler
{
public:
    NetworkHttpRestHandler(const ucf::service::network::http::HttpRestRequest& restRequest, const HttpRestResponseCallbackFunc& restResponseCallback);
    NetworkHttpRestHandler(const ucf::service::network::http::NetworkHttpRestHandler&) = delete;
    NetworkHttpRestHandler(ucf::service::network::http::NetworkHttpRestHandler&&) = delete;
    NetworkHttpRestHandler& operator=(const ucf::service::network::http::NetworkHttpRestHandler&) = delete;
    NetworkHttpRestHandler& operator=(ucf::service::network::http::NetworkHttpRestHandler&&) = delete;
    ~NetworkHttpRestHandler();
public:
    virtual const ucf::agents::network::http::NetworkHttpRequest& getHttpRequest() const override;
    virtual void setResponseHeader(int statusCode, const ucf::agents::network::http::NetworkHttpHeaders& headers, std::optional<ucf::agents::network::http::ResponseErrorStruct> errorData) override;
    virtual void appendResponseBody(const ucf::agents::network::http::ByteBuffer& buffer, bool isFinished) override;
    virtual void completeResponse(const ucf::agents::network::http::HttpResponseMetrics& metrics) override;
    virtual bool shouldRedirectRequest() const override;
    virtual void prepareRedirectRequest() override;
    
    virtual bool shouldRetryRequest() const override;
    virtual int getRetryAfterMillSecs() const override;
    virtual void prepareRetryRequest() override;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}