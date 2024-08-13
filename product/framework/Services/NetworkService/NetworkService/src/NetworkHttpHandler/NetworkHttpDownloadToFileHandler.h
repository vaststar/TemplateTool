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
class HttpDownloadToFileRequest;

class NetworkHttpDownloadToFileHandler final: public INetworkHttpHandler
{
public:
    NetworkHttpDownloadToFileHandler(const ucf::service::network::http::HttpDownloadToFileRequest& restRequest, const HttpDownloadToFileResponseCallbackFunc& restResponseCallback);
    virtual ~NetworkHttpDownloadToFileHandler();
    NetworkHttpDownloadToFileHandler(const NetworkHttpDownloadToFileHandler&) = delete;
    NetworkHttpDownloadToFileHandler(NetworkHttpDownloadToFileHandler&&) = delete;
    NetworkHttpDownloadToFileHandler& operator=(const NetworkHttpDownloadToFileHandler&) = delete;
    NetworkHttpDownloadToFileHandler& operator=(NetworkHttpDownloadToFileHandler&&) = delete;
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