#pragma once

#include <string>
#include <memory>
#include <vector>
#include <optional>

#include <ucf/Services/ServiceCommonFile/ServiceExport.h>
#include <ucf/Services/NetworkService/Model/HttpDeclareTypes.h>

namespace ucf::service::network::http{
class SERVICE_EXPORT HttpRawResponse final
{
public:
    HttpRawResponse();
    ~HttpRawResponse();
    HttpRawResponse(const HttpRawResponse&) = delete;
    HttpRawResponse(HttpRawResponse&&) = delete;
    HttpRawResponse& operator=(const HttpRawResponse&) = delete;
    HttpRawResponse& operator=(HttpRawResponse&&) = delete;

    void setHttpResponseCode(int statusCode);
    int getHttpResponseCode() const;

    void setResponseHeaders(const NetworkHttpHeaders& headers);
    NetworkHttpHeaders getResponseHeaders() const;

    void setErrorData(const ResponseErrorStruct& errorData);
    std::optional<ResponseErrorStruct> getErrorData() const;

    void setResponseBody(const ByteBuffer& body);
    ByteBuffer getResponseBody() const;

private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};

}