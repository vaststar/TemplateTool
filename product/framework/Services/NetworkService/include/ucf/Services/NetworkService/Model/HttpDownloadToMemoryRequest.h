#pragma once

#include <string>
#include <memory>
#include <vector>

#include <ucf/Services/ServiceExportMacro/ServiceExport.h>
#include <ucf/Services/NetworkService/Model/HttpDeclareTypes.h>

namespace ucf::service::network::http{

class SERVICE_EXPORT HttpDownloadToMemoryRequest final
{
public:
    HttpDownloadToMemoryRequest();
    HttpDownloadToMemoryRequest(const std::string& uri, const NetworkHttpHeaders& headers, int timeoutSecs);
    HttpDownloadToMemoryRequest(const HttpDownloadToMemoryRequest&) = delete;
    HttpDownloadToMemoryRequest(HttpDownloadToMemoryRequest&&) = delete;
    HttpDownloadToMemoryRequest& operator=(const HttpDownloadToMemoryRequest&) = delete;
    HttpDownloadToMemoryRequest& operator=(HttpDownloadToMemoryRequest&&) = delete;
    ~HttpDownloadToMemoryRequest();

    std::string getRequestId() const;
    std::string getTrackingId() const;
    std::string getRequestUri() const;
    NetworkHttpHeaders getRequestHeaders() const;
    int getTimeout() const;
private:private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}