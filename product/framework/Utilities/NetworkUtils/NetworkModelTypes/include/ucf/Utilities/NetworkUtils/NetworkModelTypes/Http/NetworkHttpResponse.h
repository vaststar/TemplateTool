#pragma once

#include <memory>
#include <string>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/NetworkModelTypesExport.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpTypes.h>

namespace ucf::utilities::network::http{
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

    void setErrorData(const ResponseErrorStruct& errorData);
    std::optional<ResponseErrorStruct> getErrorData() const;

    void setResponseBody(const std::string& body);
    std::string getResponseBody() const;

    void clear();
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}