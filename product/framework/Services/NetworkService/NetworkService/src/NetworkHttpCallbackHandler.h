#pragma once

#include <memory>
#include <functional>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpTypes.h>

namespace ucf::utilities::network::http{
class NetworkHttpResponse;
using NetworkHttpResponseCallbackFunc = std::function<void(const NetworkHttpResponse& httpResponse)>;

}
namespace ucf::service::network::http{


class NetworkHttpCallbackHandler final
{
public:
    NetworkHttpCallbackHandler();
    virtual ~NetworkHttpCallbackHandler();
    NetworkHttpCallbackHandler(const NetworkHttpCallbackHandler&) = delete;
    NetworkHttpCallbackHandler(NetworkHttpCallbackHandler&&) = delete;
    NetworkHttpCallbackHandler& operator=(const NetworkHttpCallbackHandler&) = delete;
    NetworkHttpCallbackHandler& operator=(NetworkHttpCallbackHandler&&) = delete;
public:
    void setResponseCallback(const ucf::utilities::network::http::NetworkHttpResponseCallbackFunc& callbackFunc);
    void setResponseHeader(int statusCode, const ucf::utilities::network::http::NetworkHttpHeaders& headers, std::optional<ucf::utilities::network::http::ResponseErrorStruct> errorData);
    void appendResponseBody(const ucf::utilities::network::http::ByteBuffer& buffer, bool isFinished);
    void completeResponse(const ucf::utilities::network::http::HttpResponseMetrics& metrics);
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}