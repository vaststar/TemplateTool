#pragma once

#include <string>
#include <memory>
#include <vector>
#include <optional>

#include <ucf/services/NetworkService/NetworkServiceTypesExport.h>
#include <ucf/services/NetworkService/model/HttpDeclareTypes.h>

namespace ucf::service::network::http{
class NETWORK_SERVICE_TYPES_API HttpRestResponse final
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
    const NetworkHttpHeaders& getResponseHeaders() const;

    void setErrorData(const ResponseErrorStruct& errorData);
    std::optional<ResponseErrorStruct> getErrorData() const;

    void setResponseBody(const std::string& body);
    const std::string& getResponseBody() const;

private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};

}
