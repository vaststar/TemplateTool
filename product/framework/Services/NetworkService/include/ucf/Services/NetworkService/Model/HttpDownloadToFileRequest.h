#pragma once

#include <string>
#include <memory>
#include <vector>

#include <ucf/Services/ServiceExportMacro/ServiceExport.h>
#include <ucf/Services/NetworkService/Model/HttpDeclareTypes.h>

namespace ucf::service::network::http{

class SERVICE_EXPORT HttpDownloadToFileRequest final
{
public:
    HttpDownloadToFileRequest();
    HttpDownloadToFileRequest(const std::string& uri, const NetworkHttpHeaders& headers, int timeoutSecs, const std::string& downloadFilePath);
    HttpDownloadToFileRequest(const HttpDownloadToFileRequest&) = delete;
    HttpDownloadToFileRequest(HttpDownloadToFileRequest&&) = delete;
    HttpDownloadToFileRequest& operator=(const HttpDownloadToFileRequest&) = delete;
    HttpDownloadToFileRequest& operator=(HttpDownloadToFileRequest&&) = delete;
    ~HttpDownloadToFileRequest();

    std::string getRequestId() const;
    std::string getTrackingId() const;
    std::string getRequestUri() const;
    NetworkHttpHeaders getRequestHeaders() const;
    int getTimeout() const;

    std::string getDownloadFilePath() const;
private:private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}