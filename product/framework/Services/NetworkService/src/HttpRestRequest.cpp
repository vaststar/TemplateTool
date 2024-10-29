#include <ucf/Services/NetworkService/Model/HttpRestRequest.h>
#include <ucf/Utilities/UUIDUtils/UUIDUtils.h>

namespace ucf::service::network::http{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class HttpRestRequest::DataPrivate
{
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

    void setPayloadJsonString(const std::string& str){ mPayloadJson = str;}
    std::string getPayloadJsonString() const{ return mPayloadJson;}
private:
    std::string mRequestId;
    HTTPMethod mMethod;
    std::string mUri;
    NetworkHttpHeaders mHeaders;
    int mTimeoutSecs;
    std::string mTrackingId;

    std::string mPayloadJson;
};

HttpRestRequest::DataPrivate::DataPrivate()
    : mRequestId("RequestID_"+ucf::utilities::UUIDUtils::generateUUID())
    , mTrackingId("TrackindID_" + ucf::utilities::UUIDUtils::generateUUID())
{

}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start HttpRestRequest Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
HttpRestRequest::HttpRestRequest()
{

}
HttpRestRequest::HttpRestRequest(const HTTPMethod& requestMethod, const std::string& uri, const NetworkHttpHeaders& headers, const std::string& body, int timeoutSecs)
    : mDataPrivate(std::make_unique<HttpRestRequest::DataPrivate>())
{
    mDataPrivate->setMethod(requestMethod);
    mDataPrivate->setUri(uri);
    mDataPrivate->setHeaders(headers);
    mDataPrivate->setTimeoutSecs(timeoutSecs);
    mDataPrivate->setPayloadJsonString(body);
}

HttpRestRequest::~HttpRestRequest()
{

}


std::string HttpRestRequest::getRequestId() const
{
    return mDataPrivate->getRequestId();
}
std::string HttpRestRequest::getTrackingId() const
{
    return mDataPrivate->getTrackingId();
}

HTTPMethod HttpRestRequest::getRequestMethod() const
{
    return mDataPrivate->getMethod();
}

std::string HttpRestRequest::getRequestUri() const
{
    return mDataPrivate->getUri();
}

NetworkHttpHeaders HttpRestRequest::getRequestHeaders() const
{
    return mDataPrivate->getHeaders();
}

int HttpRestRequest::getTimeout() const
{
    return mDataPrivate->getTimeoutSecs();
}

size_t HttpRestRequest::getPayloadSize() const
{
    return getPayloadJsonString().size();
}

std::string HttpRestRequest::getPayloadJsonString() const
{
    return mDataPrivate->getPayloadJsonString();
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start HttpRestRequest Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}