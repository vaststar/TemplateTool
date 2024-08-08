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
class HttpDownloadToContentRequest;

class NetworkHttpDownloadToContentHandler final: public INetworkHttpHandler
{
public:
    NetworkHttpDownloadToContentHandler(const ucf::service::network::http::HttpDownloadToContentRequest& restRequest, const HttpDownloadToContentResponseCallbackFunc& restResponseCallback);
    virtual ~NetworkHttpDownloadToContentHandler();
    NetworkHttpDownloadToContentHandler(const NetworkHttpDownloadToContentHandler&) = delete;
    NetworkHttpDownloadToContentHandler(NetworkHttpDownloadToContentHandler&&) = delete;
    NetworkHttpDownloadToContentHandler& operator=(const NetworkHttpDownloadToContentHandler&) = delete;
    NetworkHttpDownloadToContentHandler& operator=(NetworkHttpDownloadToContentHandler&&) = delete;
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