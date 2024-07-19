#include <format>
#include <variant>
#include <filesystem>

#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpRequest.h>
#include <ucf/Utilities/UUIDUtils/UUIDUtils.h>

namespace ucf::utilities::network::http{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class NetworkHttpRequest::DataPrivate{
public:
    DataPrivate();

    void setRequestId(const std::string& requestId){ mRequestId = requestId;}
    std::string getRequestId() const{ return mRequestId;}

    void setMethod(const HTTPMethod& method){ mMethod = method;}
    HTTPMethod getMethod() const{ return mMethod;}

    void setUri(const std::string& uri){ mUri = uri;}
    std::string getUri() const{ return mUri;}

    void setHeaders(const NetworkHttpHeaders& headers){ mHeaders = headers;}
    NetworkHttpHeaders getHeaders() const{ return mHeaders;}

    void setTimeoutSecs(int timeout){ mTimeoutSecs = timeout;}
    int getTimeoutSecs() const{ return mTimeoutSecs;}
    
    void setTrackingId(const std::string& trackingId){ mTrackingId = trackingId;}
    std::string getTrackingId() const{ return mTrackingId;}

    void setPayloadType(const NetworkHttpPayloadType& payloadType){ mPayloadType = payloadType;}
    NetworkHttpPayloadType getPayloadType() const{ return mPayloadType;}

    void setPayloadJsonString(const std::string& str){ mPayloadJson = str;}
    std::string getPayloadJsonString() const{ return mPayloadJson;}

    void setPayloadFilePath(const std::string& filePath){ mPayloadFilePath = filePath;}
    std::string getPayloadFilePath() const{ return mPayloadFilePath;}

    void setPayloadMemoryBuffer(ByteBufferPtr buffer){ mPayloadBuffer = buffer;}
    ByteBufferPtr getPayloadMemoryBuffer() const{ return mPayloadBuffer;}

    void setProgressFunction(UploadProgressFunction progressFunc){ mUploadProgressFunction = progressFunc;}
    UploadProgressFunction getProgressFunction() const{ return mUploadProgressFunction;}

    void clear();
private:
    std::string mRequestId;
    HTTPMethod mMethod;
    std::string mUri;
    NetworkHttpHeaders mHeaders;
    int mTimeoutSecs;
    std::string mTrackingId;

    NetworkHttpPayloadType mPayloadType;
    std::string mPayloadJson;
    std::string mPayloadFilePath;
    ByteBufferPtr mPayloadBuffer;
    UploadProgressFunction mUploadProgressFunction;
};

NetworkHttpRequest::DataPrivate::DataPrivate()
    : mRequestId("RequestID_"+ucf::utilities::UUIDUtils::generateUUID())
    , mTrackingId("TrackindID_" + ucf::utilities::UUIDUtils::generateUUID())
    , mMethod(HTTPMethod::Unknwon)
    , mPayloadType(NetworkHttpPayloadType::None)
    , mTimeoutSecs(0)
{

}

void NetworkHttpRequest::DataPrivate::clear()
{
    mRequestId.clear();
    mTrackingId.clear();
    mMethod = HTTPMethod::Unknwon;
    mUri.clear();
    mHeaders.clear();
    mTimeoutSecs = 0;
    mTrackingId.clear();
    mPayloadJson.clear();
    mPayloadFilePath.clear();
    mPayloadBuffer.reset();
    mUploadProgressFunction = nullptr;
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start NetworkHttpRequest Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
NetworkHttpRequest::NetworkHttpRequest()
    :mDataPrivate(std::make_unique<DataPrivate>())
{

}

NetworkHttpRequest::~NetworkHttpRequest()
{

}

void NetworkHttpRequest::setRequestId(const std::string& requestId)
{
    mDataPrivate->setRequestId(requestId);
}

std::string NetworkHttpRequest::getRequestId() const
{
    return mDataPrivate->getRequestId();
}

void NetworkHttpRequest::setTrackingId(const std::string& trackingId)
{
    mDataPrivate->setTrackingId(trackingId);
}

std::string NetworkHttpRequest::getTrackingId() const
{
    return mDataPrivate->getTrackingId();
}

void NetworkHttpRequest::setRequestMethod(const HTTPMethod& httpMethod)
{
    mDataPrivate->setMethod(httpMethod);
}

HTTPMethod NetworkHttpRequest::getRequestMethod() const
{
    return mDataPrivate->getMethod();
}

void NetworkHttpRequest::setRequestUri(const std::string& uri)
{
    mDataPrivate->setUri(uri);
}

std::string NetworkHttpRequest::getRequestUri() const
{
    return mDataPrivate->getUri();
}

void NetworkHttpRequest::setRequestHeaders(const NetworkHttpHeaders& headers)
{
    mDataPrivate->setHeaders(headers);
}

NetworkHttpHeaders NetworkHttpRequest::getRequestHeaders() const
{
    return mDataPrivate->getHeaders();
}


void NetworkHttpRequest::setTimeout(int timeoutSecs)
{
    mDataPrivate->setTimeoutSecs(timeoutSecs);
}

int NetworkHttpRequest::getTimeout() const
{
    return mDataPrivate->getTimeoutSecs();
}


NetworkHttpPayloadType NetworkHttpRequest::getPayloadType() const
{
    return mDataPrivate->getPayloadType();
}

size_t NetworkHttpRequest::getPayloadSize() const
{
    switch (getPayloadType())
    {
    case NetworkHttpPayloadType::Json:
        return getPayloadJsonString().size();
    case NetworkHttpPayloadType::Memory:
        return getPayloadMemoryBuffer()->size();
    case NetworkHttpPayloadType::File:
        return std::filesystem::file_size(getPayloadFilePath());
    default:
        return 0;
    }
}

void NetworkHttpRequest::setPayloadJsonString(const std::string& jsonStr)
{
    mDataPrivate->setPayloadType(NetworkHttpPayloadType::Json);
    mDataPrivate->setPayloadJsonString(jsonStr);
}

std::string NetworkHttpRequest::getPayloadJsonString() const
{
    return mDataPrivate->getPayloadJsonString();
}

void NetworkHttpRequest::setPayloadFilePath(const std::string& filePath)
{
    mDataPrivate->setPayloadType(NetworkHttpPayloadType::File);
    mDataPrivate->setPayloadFilePath(filePath);
}

std::string NetworkHttpRequest::getPayloadFilePath() const
{
    return mDataPrivate->getPayloadFilePath();
}

void NetworkHttpRequest::setPayloadMemoryBuffer(ByteBufferPtr buffer)
{
    mDataPrivate->setPayloadType(NetworkHttpPayloadType::Memory);
    mDataPrivate->setPayloadMemoryBuffer(buffer);
}

ByteBufferPtr NetworkHttpRequest::getPayloadMemoryBuffer() const
{
    return mDataPrivate->getPayloadMemoryBuffer();
}

void NetworkHttpRequest::setProgressFunction(UploadProgressFunction progressFunc)
{
    mDataPrivate->setProgressFunction(progressFunc);
}

UploadProgressFunction NetworkHttpRequest::getProgressFunction() const
{
    return mDataPrivate->getProgressFunction();
}

void NetworkHttpRequest::clear()
{
    mDataPrivate->clear();
}

std::string NetworkHttpRequest::toString() const
{
    auto getMethodString = [this]() {
        switch (getRequestMethod())
        {
        case HTTPMethod::GET:
            return "GET";
        case HTTPMethod::POST:
            return "POST";
        case HTTPMethod::HEAD:
            return "HEAD";
        case HTTPMethod::PUT:
            return "PUT";
        case HTTPMethod::DEL:
            return "DEL";
        case HTTPMethod::PATCH:
            return "PATCH";
        case HTTPMethod::OPTIONS:
            return "OPTIONS";
        default:
            return "UNKNOWN";
        }};
    return "NetworkHttpRequest, {" + std::format("\"Method\": \"{}\", \"URI\": \"{}\", \"RequestId\": \"{}\", \"TrackingId\": \"{}\"", getMethodString(), getRequestUri(), getRequestId(), getTrackingId()) + "}";
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish NetworkHttpRequest Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}