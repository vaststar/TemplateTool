#pragma once

#include <string>
#include <memory>
#include <vector>

#include <ucf/Services/ServiceCommonFile/ServiceExport.h>
#include <ucf/Services/NetworkService/Model/HttpDeclareTypes.h>

namespace ucf::service::network::http{

class SERVICE_EXPORT HttpRawRequest final
{
public:
    HttpRawRequest();
    HttpRawRequest(const HTTPMethod& requestMethod, const std::string& uri, const NetworkHttpHeaders& headers, const std::string& body, int timeoutSecs);
    HttpRawRequest(const HttpRawRequest&) = delete;
    HttpRawRequest(HttpRawRequest&&) = delete;
    HttpRawRequest& operator=(const HttpRawRequest&) = delete;
    HttpRawRequest& operator=(HttpRawRequest&&) = delete;
    ~HttpRawRequest();

    std::string getRequestId() const;
    std::string getTrackingId() const;
    HTTPMethod getRequestMethod() const;
    std::string getRequestUri() const;
    NetworkHttpHeaders getRequestHeaders() const;
    int getTimeout() const;

    size_t getPayloadSize() const;
    std::string getPayloadString() const;
private:private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}