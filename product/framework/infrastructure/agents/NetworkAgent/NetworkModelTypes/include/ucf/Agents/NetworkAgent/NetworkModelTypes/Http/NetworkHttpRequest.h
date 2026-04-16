#pragma once

#include <memory>
#include <string>

#include <ucf/Agents/AgentsCommonFile/AgentsExport.h>
#include <ucf/Agents/NetworkAgent/NetworkModelTypes/Http/NetworkHttpTypes.h>

namespace ucf::agents::network::http{

/**
 * @brief HTTP request object containing all data needed to make an HTTP request.
 * 
 * This class encapsulates request parameters including method, URI, headers,
 * timeout, and payload. Request ID and Tracking ID are auto-generated if not set.
 * 
 * @note This class is non-copyable and non-movable due to PIMPL implementation.
 */
class Agents_EXPORT NetworkHttpRequest final
{
public:
    NetworkHttpRequest();
    ~NetworkHttpRequest();
    NetworkHttpRequest(const NetworkHttpRequest&) = delete;
    NetworkHttpRequest(NetworkHttpRequest&&) = delete;
    NetworkHttpRequest& operator=(const NetworkHttpRequest&) = delete;
    NetworkHttpRequest& operator=(NetworkHttpRequest&&) = delete;

    //requestID could be autofilled
    void setRequestId(const std::string& requestId);
    const std::string& getRequestId() const;

    //trackingID could be autofilled
    void setTrackingId(const std::string& trackingId);
    const std::string& getTrackingId() const;

    void setRequestMethod(const ucf::agents::network::http::HTTPMethod& httpMethod);
    ucf::agents::network::http::HTTPMethod getRequestMethod() const;

    void setRequestUri(const std::string& requestUri);
    const std::string& getRequestUri() const;

    void setRequestHeaders(const NetworkHttpHeaders& headers);
    const NetworkHttpHeaders& getRequestHeaders() const;

    void setTimeout(int timeout);
    int getTimeout() const;

    NetworkHttpPayloadType getPayloadType() const;
    size_t getPayloadSize() const;

    void setPayloadString(const std::string& stringPayload);
    const std::string& getPayloadString() const;

    void setPayloadFilePath(const std::string& filePath);
    const std::string& getPayloadFilePath() const;

    void setPayloadMemoryBuffer(ByteBufferPtr byteBuffer);
    ByteBufferPtr getPayloadMemoryBuffer() const;

    void setProgressFunction(UploadProgressFunction progressFunc);
    UploadProgressFunction getProgressFunction() const;

    std::string toString() const;
    void clear();
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}