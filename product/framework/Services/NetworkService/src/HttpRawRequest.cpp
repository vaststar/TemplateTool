#include <ucf/Services/NetworkService/Model/HttpRawRequest.h>
#include <ucf/Utilities/UUIDUtils/UUIDUtils.h>

namespace ucf::service::network::http{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class HttpRawRequest::DataPrivate
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

    void setPayloadString(const std::string& str){ mPayload = str;}
    std::string getPayloadString() const{ return mPayload;}
private:
    std::string mRequestId;
    HTTPMethod mMethod;
    std::string mUri;
    NetworkHttpHeaders mHeaders;
    int mTimeoutSecs;
    std::string mTrackingId;

    std::string mPayload;
};

HttpRawRequest::DataPrivate::DataPrivate()
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
////////////////////Start HttpRawRequest Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
HttpRawRequest::HttpRawRequest()
{

}
HttpRawRequest::HttpRawRequest(const HTTPMethod& requestMethod, const std::string& uri, const NetworkHttpHeaders& headers, const std::string& body, int timeoutSecs)
    : mDataPrivate(std::make_unique<HttpRawRequest::DataPrivate>())
{
    mDataPrivate->setMethod(requestMethod);
    mDataPrivate->setUri(uri);
    mDataPrivate->setHeaders(headers);
    mDataPrivate->setTimeoutSecs(timeoutSecs);
    mDataPrivate->setPayloadString(body);
}

HttpRawRequest::~HttpRawRequest()
{

}


std::string HttpRawRequest::getRequestId() const
{
    return mDataPrivate->getRequestId();
}
std::string HttpRawRequest::getTrackingId() const
{
    return mDataPrivate->getTrackingId();
}

HTTPMethod HttpRawRequest::getRequestMethod() const
{
    return mDataPrivate->getMethod();
}

std::string HttpRawRequest::getRequestUri() const
{
    return mDataPrivate->getUri();
}

NetworkHttpHeaders HttpRawRequest::getRequestHeaders() const
{
    return mDataPrivate->getHeaders();
}

int HttpRawRequest::getTimeout() const
{
    return mDataPrivate->getTimeoutSecs();
}

size_t HttpRawRequest::getPayloadSize() const
{
    return getPayloadString().size();
}

std::string HttpRawRequest::getPayloadString() const
{
    return mDataPrivate->getPayloadString();
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start HttpRawRequest Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}