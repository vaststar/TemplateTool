#pragma once

#include <memory>
#include <functional>
#include <ucf/Services/NetworkService/NetworkModelTypes/Http/NetworkHttpTypes.h>
namespace ucf::service::network::http{

class NetworkHttpResponse;
using NetworkHttpResponseCallbackFunc = std::function<void(const NetworkHttpResponse& httpResponse)>;

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
    void setResponseCallback(const NetworkHttpResponseCallbackFunc& callbackFunc);
    void setResponseHeader(int statusCode, const NetworkHttpHeaders& headers, std::optional<ResponseErrorStruct> errorData);
    void appendResponseBody(const ByteBuffer& buffer, bool isFinished);
    void completeResponse(const HttpResponseMetrics& metrics);
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}