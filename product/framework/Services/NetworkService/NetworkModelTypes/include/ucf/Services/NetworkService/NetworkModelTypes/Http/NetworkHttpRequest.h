#pragma once

#include <memory>
#include <map>
#include <string>
#include <ucf/Services/NetworkService/NetworkModelTypes/NetworkModelTypesExport.h>
#include <ucf/Services/NetworkService/NetworkModelTypes/Http/NetworkHttpTypes.h>

namespace ucf::network::http{
class NETWORKTYPE_EXPORT NetworkHttpRequest final
{
public:
    NetworkHttpRequest(const HTTPMethod& method, const std::string& uri, const std::map<std::string, std::string>& headers, const std::string& payload);
    ~NetworkHttpRequest();
    NetworkHttpRequest(const NetworkHttpRequest&) = delete;
    NetworkHttpRequest(NetworkHttpRequest&&) = delete;
    NetworkHttpRequest& operator=(const NetworkHttpRequest&) = delete;
    NetworkHttpRequest& operator=(NetworkHttpRequest&&) = delete;

    std::string getRequestId() const;
    HTTPMethod getRequestMethod() const;
    std::string getRequestUri() const;
    std::map<std::string, std::string> getRequestHeaders() const;
    std::string getRequestPayload() const;

    void setTrackingId(const std::string& trackingId);
    std::string getTrackingId() const;

    std::string toString() const;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}