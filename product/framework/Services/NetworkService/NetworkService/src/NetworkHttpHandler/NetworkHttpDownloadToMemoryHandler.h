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
class HttpDownloadToMemoryRequest;

class NetworkHttpDownloadToMemoryHandler final: public INetworkHttpHandler
{
public:
    NetworkHttpDownloadToMemoryHandler(const ucf::service::network::http::HttpDownloadToMemoryRequest& restRequest, const HttpDownloadToMemoryResponseCallbackFunc& restResponseCallback);
    virtual ~NetworkHttpDownloadToMemoryHandler();
    NetworkHttpDownloadToMemoryHandler(const NetworkHttpDownloadToMemoryHandler&) = delete;
    NetworkHttpDownloadToMemoryHandler(NetworkHttpDownloadToMemoryHandler&&) = delete;
    NetworkHttpDownloadToMemoryHandler& operator=(const NetworkHttpDownloadToMemoryHandler&) = delete;
    NetworkHttpDownloadToMemoryHandler& operator=(NetworkHttpDownloadToMemoryHandler&&) = delete;
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