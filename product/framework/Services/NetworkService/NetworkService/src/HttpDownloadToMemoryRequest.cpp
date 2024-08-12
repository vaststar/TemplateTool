#include <ucf/Services/NetworkService/Model/HttpDownloadToMemoryRequest.h>
#include <ucf/Utilities/UUIDUtils/UUIDUtils.h>

namespace ucf::service::network::http{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class HttpDownloadToMemoryRequest::DataPrivate
{
public:
    DataPrivate();
    void setRequestId(const std::string& requestId){ mRequestId = requestId;}
    std::string getRequestId() const{ return mRequestId;}

    void setUri(const std::string& uri){ mUri = uri;}
    std::string getUri() const{ return mUri;}

    void setHeaders(const NetworkHttpHeaders& headers){ mHeaders = headers;}
    NetworkHttpHeaders getHeaders() const{ return mHeaders;}

    void setTimeoutSecs(int timeout){ mTimeoutSecs = timeout;}
    int getTimeoutSecs() const{ return mTimeoutSecs;}
    
    void setTrackingId(const std::string& trackingId){ mTrackingId = trackingId;}
    std::string getTrackingId() const{ return mTrackingId;}

private:
    std::string mRequestId;
    std::string mUri;
    NetworkHttpHeaders mHeaders;
    int mTimeoutSecs;
    std::string mTrackingId;
};

HttpDownloadToMemoryRequest::DataPrivate::DataPrivate()
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
////////////////////Start HttpDownloadToMemoryRequest Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
HttpDownloadToMemoryRequest::HttpDownloadToMemoryRequest()
{

}
HttpDownloadToMemoryRequest::HttpDownloadToMemoryRequest( const std::string& uri, const NetworkHttpHeaders& headers, int timeoutSecs)
    : mDataPrivate(std::make_unique<HttpDownloadToMemoryRequest::DataPrivate>())
{
    mDataPrivate->setUri(uri);
    mDataPrivate->setHeaders(headers);
    mDataPrivate->setTimeoutSecs(timeoutSecs);
}

HttpDownloadToMemoryRequest::~HttpDownloadToMemoryRequest()
{

}


std::string HttpDownloadToMemoryRequest::getRequestId() const
{
    return mDataPrivate->getRequestId();
}
std::string HttpDownloadToMemoryRequest::getTrackingId() const
{
    return mDataPrivate->getTrackingId();
}

std::string HttpDownloadToMemoryRequest::getRequestUri() const
{
    return mDataPrivate->getUri();
}

NetworkHttpHeaders HttpDownloadToMemoryRequest::getRequestHeaders() const
{
    return mDataPrivate->getHeaders();
}

int HttpDownloadToMemoryRequest::getTimeout() const
{
    return mDataPrivate->getTimeoutSecs();
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start HttpDownloadToMemoryRequest Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}