#pragma once

#include <string>
#include <memory>
#include <vector>

#include <ucf/Services/ServiceCommonFile/ServiceExport.h>
#include <ucf/Services/NetworkService/Model/HttpDeclareTypes.h>

namespace ucf::service::network::http{
class SERVICE_EXPORT HttpRestRequest final
{
public:
    HttpRestRequest();
    HttpRestRequest(const HTTPMethod& requestMethod, const std::string& uri, const NetworkHttpHeaders& headers, const std::string& body, int timeoutSecs);
    HttpRestRequest(const HttpRestRequest&) = delete;
    HttpRestRequest(HttpRestRequest&&) = delete;
    HttpRestRequest& operator=(const HttpRestRequest&) = delete;
    HttpRestRequest& operator=(HttpRestRequest&&) = delete;
    ~HttpRestRequest();

    std::string getRequestId() const;
    std::string getTrackingId() const;
    HTTPMethod getRequestMethod() const;
    std::string getRequestUri() const;
    NetworkHttpHeaders getRequestHeaders() const;
    int getTimeout() const;

    size_t getPayloadSize() const;
    std::string getPayloadJsonString() const;
private:private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}