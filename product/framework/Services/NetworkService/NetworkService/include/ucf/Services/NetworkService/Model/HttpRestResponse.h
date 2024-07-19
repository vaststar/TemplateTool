#pragma once

#include <string>
#include <memory>
#include <vector>
#include <optional>

#include <ucf/Services/ServiceCommonFile/ServiceExport.h>

namespace ucf::service::network::http{
using HttpResponseHeaders = std::vector<std::pair<std::string, std::string>>;
enum class ResponseErrorType
{
    NoError,
    DNSError,
    SocketError,
    TLSError,
    TimeoutError,
    CancelledError,
    OtherError,
    UnHandledError
};

struct ResponseErrorStruct{
    int errorCode{ 0 };
    ResponseErrorType errorType{ResponseErrorType::NoError};
    std::string errorDescription;
};
class SERVICE_EXPORT HttpRestResponse final
{
public:
    HttpRestResponse();
    ~HttpRestResponse();
    HttpRestResponse(const HttpRestResponse&) = delete;
    HttpRestResponse(HttpRestResponse&&) = delete;
    HttpRestResponse& operator=(const HttpRestResponse&) = delete;
    HttpRestResponse& operator=(HttpRestResponse&&) = delete;

    void setHttpResponseCode(int statusCode);
    int getHttpResponseCode() const;

    void setResponseHeaders(const HttpResponseHeaders& headers);
    HttpResponseHeaders getResponseHeaders() const;

    void setErrorData(const ResponseErrorStruct& errorData);
    std::optional<ResponseErrorStruct> getErrorData() const;

    void setResponseBody(const std::string& body);
    std::string getResponseBody() const;

private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};

}