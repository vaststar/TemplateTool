#pragma once

#include <memory>
#include <map>
#include <string>
#include <ucf/Services/ServiceCommonFile/ServiceExport.h>
#include <ucf/Services/NetworkService/Http/NetworkHttpTypes.h>

namespace ucf::network::http{
class SERVICE_EXPORT NetworkHttpRequest
{
public:
    NetworkHttpRequest(const HTTPMethod& method, const std::string& uri, const std::map<std::string, std::string>& headers, const std::string& payload);
    HTTPMethod getRequestMethod() const;
    std::string getRequestUri() const;
    std::map<std::string, std::string> getRequestHeaders() const;
    std::string getRequestPayload() const;
private:
    class DataPrivate;
    std::shared_ptr<DataPrivate> mDataPrivate;
};
}