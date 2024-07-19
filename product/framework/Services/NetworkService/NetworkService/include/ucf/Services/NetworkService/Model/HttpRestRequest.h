#pragma once

#include <string>
#include <memory>
#include <vector>

#include <ucf/Services/ServiceCommonFile/ServiceExport.h>

namespace ucf::service::network::http{
using HttpRequestHeaders = std::vector<std::pair<std::string, std::string>>;

enum class HTTPMethod{
    GET,
    POST,
    HEAD,
    PUT,
    DEL,
    PATCH,
    OPTIONS
};

class SERVICE_EXPORT HttpRestRequest final
{
public:
    HttpRestRequest();
    HttpRestRequest(const HTTPMethod& requestMethod, const std::string& uri, const HttpRequestHeaders& headers, const std::string& body, int timeoutSecs);
    ~HttpRestRequest();

    std::string getRequestId() const;
    std::string getTrackingId() const;
    HTTPMethod getRequestMethod() const;
    std::string getRequestUri() const;
    HttpRequestHeaders getRequestHeaders() const;
    int getTimeout() const;

    size_t getPayloadSize() const;
    std::string getPayloadJsonString() const;
private:private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}