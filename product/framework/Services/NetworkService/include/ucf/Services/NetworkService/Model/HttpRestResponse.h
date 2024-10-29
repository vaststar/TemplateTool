#pragma once

#include <string>
#include <memory>
#include <vector>
#include <optional>

#include <ucf/Services/ServiceCommonFile/ServiceExport.h>
#include <ucf/Services/NetworkService/Model/HttpDeclareTypes.h>

namespace ucf::service::network::http{
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

    void setResponseHeaders(const NetworkHttpHeaders& headers);
    NetworkHttpHeaders getResponseHeaders() const;

    void setErrorData(const ResponseErrorStruct& errorData);
    std::optional<ResponseErrorStruct> getErrorData() const;

    void setResponseBody(const std::string& body);
    std::string getResponseBody() const;

private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};

}