#pragma once

#include <string>
#include <memory>
#include <vector>

#include <ucf/Services/ServiceCommonFile/ServiceExport.h>
#include <ucf/Services/NetworkService/Model/HttpDeclareTypes.h>

namespace ucf::service::network::http{

class SERVICE_EXPORT HttpDownloadToContentRequest final
{
public:
    HttpDownloadToContentRequest();
    HttpDownloadToContentRequest(const std::string& uri, const NetworkHttpHeaders& headers, int timeoutSecs);
    HttpDownloadToContentRequest(const HttpDownloadToContentRequest&) = delete;
    HttpDownloadToContentRequest(HttpDownloadToContentRequest&&) = delete;
    HttpDownloadToContentRequest& operator=(const HttpDownloadToContentRequest&) = delete;
    HttpDownloadToContentRequest& operator=(HttpDownloadToContentRequest&&) = delete;
    ~HttpDownloadToContentRequest();

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