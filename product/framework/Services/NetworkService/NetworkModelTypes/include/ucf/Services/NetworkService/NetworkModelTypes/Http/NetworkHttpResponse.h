#pragma once

#include <memory>
#include <string>
#include <ucf/Services/NetworkService/NetworkModelTypes/NetworkModelTypesExport.h>
#include <ucf/Services/NetworkService/NetworkModelTypes/Http/NetworkHttpTypes.h>

namespace ucf::service::network::http{
class NETWORKTYPE_EXPORT NetworkHttpResponse final
{
public:
    NetworkHttpResponse();
    ~NetworkHttpResponse();
    NetworkHttpResponse(const NetworkHttpResponse&) = delete;
    NetworkHttpResponse(NetworkHttpResponse&&) = delete;
    NetworkHttpResponse& operator=(const NetworkHttpResponse&) = delete;
    NetworkHttpResponse& operator=(NetworkHttpResponse&&) = delete;

    void setHttpResponseCode(int statusCode);
    int getHttpResponseCode() const;

    void setResponseHeaders(const NetworkHttpHeaders& headers);
    NetworkHttpHeaders getResponseHeaders() const;

private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}