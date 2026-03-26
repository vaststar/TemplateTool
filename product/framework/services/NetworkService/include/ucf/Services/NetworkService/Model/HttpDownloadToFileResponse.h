#pragma once

#include <string>
#include <memory>
#include <vector>
#include <optional>

#include <ucf/Services/ServiceExportMacro/ServiceExport.h>
#include <ucf/Services/NetworkService/Model/HttpDeclareTypes.h>

namespace ucf::service::network::http{
class SERVICE_EXPORT HttpDownloadToFileResponse final
{
public:
    HttpDownloadToFileResponse();
    ~HttpDownloadToFileResponse();
    HttpDownloadToFileResponse(const HttpDownloadToFileResponse&) = delete;
    HttpDownloadToFileResponse(HttpDownloadToFileResponse&&) = delete;
    HttpDownloadToFileResponse& operator=(const HttpDownloadToFileResponse&) = delete;
    HttpDownloadToFileResponse& operator=(HttpDownloadToFileResponse&&) = delete;

    void setHttpResponseCode(int statusCode);
    int getHttpResponseCode() const;

    void setResponseHeaders(const NetworkHttpHeaders& headers);
    const NetworkHttpHeaders& getResponseHeaders() const;

    void setErrorData(const ResponseErrorStruct& errorData);
    std::optional<ResponseErrorStruct> getErrorData() const;

    void appendResponseBody(const ByteBuffer& body);
    size_t getCurrentSize() const;
    size_t getTotalSize() const;

    bool isFinished() const;
    void setFinished();

    void clear();
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};

}