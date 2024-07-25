#pragma once

#include <string>
#include <memory>
#include <vector>

#include <ucf/Services/ServiceCommonFile/ServiceExport.h>
#include <ucf/Services/NetworkService/Model/HttpDeclareTypes.h>

namespace ucf::service::network::http{

class SERVICE_EXPORT HttpDataTransferRequest final
{
public:
    HttpDataTransferRequest();
    HttpDataTransferRequest(const HTTPMethod& requestMethod, const std::string& uri, const NetworkHttpHeaders& headers, const std::string& body, int timeoutSecs);
    HttpDataTransferRequest(const HttpDataTransferRequest&) = delete;
    HttpDataTransferRequest(HttpDataTransferRequest&&) = delete;
    HttpDataTransferRequest& operator=(const HttpDataTransferRequest&) = delete;
    HttpDataTransferRequest& operator=(HttpDataTransferRequest&&) = delete;
    ~HttpDataTransferRequest();

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