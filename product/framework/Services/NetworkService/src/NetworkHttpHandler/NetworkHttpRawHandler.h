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
    virtual const ucf::utilities::network::http::NetworkHttpRequest& getHttpRequest() const override;
    virtual void setResponseHeader(int statusCode, const ucf::utilities::network::http::NetworkHttpHeaders& headers, std::optional<ucf::utilities::network::http::ResponseErrorStruct> errorData) override;
    virtual void appendResponseBody(const ucf::utilities::network::http::ByteBuffer& buffer, bool isFinished) override;
    virtual void completeResponse(const ucf::utilities::network::http::HttpResponseMetrics& metrics) override;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}