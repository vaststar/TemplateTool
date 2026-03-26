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
class HttpDownloadToMemoryRequest;

class NetworkHttpDownloadToMemoryHandler final: public INetworkHttpHandler
{
public:
    NetworkHttpDownloadToMemoryHandler(const ucf::service::network::http::HttpDownloadToMemoryRequest& restRequest, const HttpDownloadToMemoryResponseCallbackFunc& restResponseCallback);
    NetworkHttpDownloadToMemoryHandler(const ucf::service::network::http::NetworkHttpDownloadToMemoryHandler&) = delete;
    NetworkHttpDownloadToMemoryHandler(ucf::service::network::http::NetworkHttpDownloadToMemoryHandler&&) = delete;
    NetworkHttpDownloadToMemoryHandler& operator=(const ucf::service::network::http::NetworkHttpDownloadToMemoryHandler&) = delete;
    NetworkHttpDownloadToMemoryHandler& operator=(ucf::service::network::http::NetworkHttpDownloadToMemoryHandler&&) = delete;
    ~NetworkHttpDownloadToMemoryHandler();
public:
    virtual const ucf::agents::network::http::NetworkHttpRequest& getHttpRequest() const override;
    virtual void setResponseHeader(int statusCode, const ucf::agents::network::http::NetworkHttpHeaders& headers, std::optional<ucf::agents::network::http::ResponseErrorStruct> errorData) override;
    virtual void appendResponseBody(const ucf::agents::network::http::ByteBuffer& buffer, bool isFinished) override;
    virtual void completeResponse(const ucf::agents::network::http::HttpResponseMetrics& metrics) override;

    virtual bool shouldRedirectRequest() const override;
    virtual void prepareRedirectRequest() override;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}