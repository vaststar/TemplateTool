#pragma once

#include <string>
#include <memory>
#include <vector>

#include <ucf/services/NetworkService/NetworkServiceTypesExport.h>
#include <ucf/services/NetworkService/model/HttpDeclareTypes.h>

namespace ucf::service::network::http{

class NETWORK_SERVICE_TYPES_API HttpDownloadToMemoryRequest final
{
public:
    HttpDownloadToMemoryRequest();
    HttpDownloadToMemoryRequest(const std::string& uri, const NetworkHttpHeaders& headers, int timeoutSecs);
    HttpDownloadToMemoryRequest(const HttpDownloadToMemoryRequest&) = delete;
    HttpDownloadToMemoryRequest(HttpDownloadToMemoryRequest&&) = delete;
    HttpDownloadToMemoryRequest& operator=(const HttpDownloadToMemoryRequest&) = delete;
    HttpDownloadToMemoryRequest& operator=(HttpDownloadToMemoryRequest&&) = delete;
    ~HttpDownloadToMemoryRequest();

    const std::string& getRequestId() const;
    const std::string& getTrackingId() const;
    const std::string& getRequestUri() const;
    const NetworkHttpHeaders& getRequestHeaders() const;
    int getTimeout() const;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}
