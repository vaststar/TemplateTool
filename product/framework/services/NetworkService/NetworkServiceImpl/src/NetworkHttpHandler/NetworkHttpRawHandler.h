#pragma once

#include <memory>
#include <functional>
#include <ucf/Infrastructure/NetworkClient/NetworkModelTypes/Http/NetworkHttpTypes.h>
#include <ucf/Services/NetworkService/Model/HttpDeclareTypes.h>

#include "NetworkHttpHandler/INetworkHttpHandler.h"

namespace ucf::infrastructure::network::http{
class NetworkHttpRequest;

}
namespace ucf::service::network::http{
class HttpRawRequest;


class NetworkHttpRawHandler final: public INetworkHttpHandler
{
public:
    NetworkHttpRawHandler(const ucf::service::network::http::HttpRawRequest& restRequest, const HttpRawResponseCallbackFunc& restResponseCallback);
    NetworkHttpRawHandler(const ucf::service::network::http::NetworkHttpRawHandler&) = delete;
    NetworkHttpRawHandler(ucf::service::network::http::NetworkHttpRawHandler&&) = delete;
    NetworkHttpRawHandler& operator=(const ucf::service::network::http::NetworkHttpRawHandler&) = delete;
    NetworkHttpRawHandler& operator=(ucf::service::network::http::NetworkHttpRawHandler&&) = delete;
    ~NetworkHttpRawHandler();
public:
    virtual const ucf::infrastructure::network::http::NetworkHttpRequest& getHttpRequest() const override;
    virtual void setResponseHeader(int statusCode, const ucf::infrastructure::network::http::NetworkHttpHeaders& headers, std::optional<ucf::infrastructure::network::http::ResponseErrorStruct> errorData) override;
    virtual void appendResponseBody(const ucf::infrastructure::network::http::ByteBuffer& buffer, bool isFinished) override;
    virtual void completeResponse(const ucf::infrastructure::network::http::HttpResponseMetrics& metrics) override;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}