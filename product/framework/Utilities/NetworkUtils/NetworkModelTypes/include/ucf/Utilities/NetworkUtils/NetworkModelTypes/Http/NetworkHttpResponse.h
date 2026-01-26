#pragma once

#include <memory>
#include <string>
#include <optional>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/NetworkModelTypesExport.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpTypes.h>

namespace ucf::utilities::network::http{

/**
 * @brief HTTP response object containing the result of an HTTP request.
 * 
 * This class encapsulates response data including status code, headers,
 * body content, and error information if the request failed.
 * 
 * @note This class is non-copyable and non-movable due to PIMPL implementation.
 */
class NETWORKTYPE_EXPORT NetworkHttpResponse final
{
public:
    NetworkHttpResponse();
    ~NetworkHttpResponse();
    NetworkHttpResponse(const NetworkHttpResponse&) = delete;
    NetworkHttpResponse(NetworkHttpResponse&&) = delete;
    NetworkHttpResponse& operator=(const NetworkHttpResponse&) = delete;
    NetworkHttpResponse& operator=(NetworkHttpResponse&&) = delete;

    void setHttpResponseCode(int statusCode);
    int getHttpResponseCode() const;

    void setResponseHeaders(const NetworkHttpHeaders& headers);
    const NetworkHttpHeaders& getResponseHeaders() const;

    void setErrorData(const ResponseErrorStruct& errorData);
    std::optional<ResponseErrorStruct> getErrorData() const;

    void appendResponseBody(const ByteBuffer& buffer);
    void setResponseBody(const ByteBuffer& buffer);
    const ByteBuffer& getResponseBody() const;

    std::optional<std::string> getHeaderValue(const std::string& key) const;

    void clear();
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}