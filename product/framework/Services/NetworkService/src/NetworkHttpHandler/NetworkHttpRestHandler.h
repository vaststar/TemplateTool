#pragma once

#include <memory>
#include <functional>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpTypes.h>
#include <ucf/Services/NetworkService/Model/HttpDeclareTypes.h>

#include "NetworkHttpHandler/INetworkHttpHandler.h"

namespace ucf::utilities::network::http{
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
    virtual const ucf::utilities::network::http::NetworkHttpRequest& getHttpRequest() const override;
    virtual void setResponseHeader(int statusCode, const ucf::utilities::network::http::NetworkHttpHeaders& headers, std::optional<ucf::utilities::network::http::ResponseErrorStruct> errorData) override;
    virtual void appendResponseBody(const ucf::utilities::network::http::ByteBuffer& buffer, bool isFinished) override;
    virtual void completeResponse(const ucf::utilities::network::http::HttpResponseMetrics& metrics) override;
    virtual bool shouldRedirectRequest() const override;
    virtual void prepareRedirectRequest() override;
    
    virtual bool shoudRetryRequest() const override;
    virtual int getRetryAfterMillSecs() const override;
    virtual void prepareRetryRequest() override;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}