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

    const std::string& getRequestId() const;
    const std::string& getTrackingId() const;
    const std::string& getRequestUri() const;
    const NetworkHttpHeaders& getRequestHeaders() const;
    int getTimeout() const;

    const std::string& getDownloadFilePath() const;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}