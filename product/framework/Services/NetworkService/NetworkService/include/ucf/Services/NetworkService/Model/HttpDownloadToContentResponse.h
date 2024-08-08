#pragma once

#include <string>
#include <memory>
#include <vector>
#include <optional>

#include <ucf/Services/ServiceCommonFile/ServiceExport.h>
#include <ucf/Services/NetworkService/Model/HttpDeclareTypes.h>

namespace ucf::service::network::http{
class SERVICE_EXPORT HttpDownloadToContentResponse final
{
public:
    HttpDownloadToContentResponse();
    ~HttpDownloadToContentResponse();
    HttpDownloadToContentResponse(const HttpDownloadToContentResponse&) = delete;
    HttpDownloadToContentResponse(HttpDownloadToContentResponse&&) = delete;
    HttpDownloadToContentResponse& operator=(const HttpDownloadToContentResponse&) = delete;
    HttpDownloadToContentResponse& operator=(HttpDownloadToContentResponse&&) = delete;

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

private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};

}