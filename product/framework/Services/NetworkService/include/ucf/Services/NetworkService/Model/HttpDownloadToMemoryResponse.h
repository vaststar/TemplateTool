#pragma once

#include <string>
#include <memory>
#include <vector>
#include <optional>

#include <ucf/Services/ServiceExportMacro/ServiceExport.h>
#include <ucf/Services/NetworkService/Model/HttpDeclareTypes.h>

namespace ucf::service::network::http{
class SERVICE_EXPORT HttpDownloadToMemoryResponse final
{
public:
    HttpDownloadToMemoryResponse();
    ~HttpDownloadToMemoryResponse();
    HttpDownloadToMemoryResponse(const HttpDownloadToMemoryResponse&) = delete;
    HttpDownloadToMemoryResponse(HttpDownloadToMemoryResponse&&) = delete;
    HttpDownloadToMemoryResponse& operator=(const HttpDownloadToMemoryResponse&) = delete;
    HttpDownloadToMemoryResponse& operator=(HttpDownloadToMemoryResponse&&) = delete;

    void setHttpResponseCode(int statusCode);
    int getHttpResponseCode() const;

    void setResponseHeaders(const NetworkHttpHeaders& headers);
    NetworkHttpHeaders getResponseHeaders() const;

    void setErrorData(const ResponseErrorStruct& errorData);
    std::optional<ResponseErrorStruct> getErrorData() const;

    void appendResponseBody(const ByteBuffer& body);
    void setResponseBody(const ByteBuffer& body);
    const ByteBuffer& getResponseBody() const;

    size_t getTotalSize() const;

    bool isFinished() const;
    void setFinished();
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};

}