#include <ucf/Services/NetworkService/Model/HttpDownloadToFileRequest.h>
#include <ucf/Utilities/UUIDUtils/UUIDUtils.h>

namespace ucf::service::network::http{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class HttpDownloadToFileRequest::DataPrivate
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

    void setDownloadFilePath(const std::string& downloadFilePath){ mDownloadFilePath = downloadFilePath;}
    std::string getDownloadFilePath() const{ return mDownloadFilePath;}
private:
    std::string mRequestId;
    std::string mUri;
    NetworkHttpHeaders mHeaders;
    int mTimeoutSecs;
    std::string mTrackingId;
    std::string mDownloadFilePath;
};

HttpDownloadToFileRequest::DataPrivate::DataPrivate()
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
////////////////////Start HttpDownloadToFileRequest Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
HttpDownloadToFileRequest::HttpDownloadToFileRequest()
{

}
HttpDownloadToFileRequest::HttpDownloadToFileRequest( const std::string& uri, const NetworkHttpHeaders& headers, int timeoutSecs, const std::string& downloadFilePath)
    : mDataPrivate(std::make_unique<HttpDownloadToFileRequest::DataPrivate>())
{
    mDataPrivate->setUri(uri);
    mDataPrivate->setHeaders(headers);
    mDataPrivate->setTimeoutSecs(timeoutSecs);
    mDataPrivate->setDownloadFilePath(downloadFilePath);
}

HttpDownloadToFileRequest::~HttpDownloadToFileRequest()
{

}

std::string HttpDownloadToFileRequest::getRequestId() const
{
    return mDataPrivate->getRequestId();
}
std::string HttpDownloadToFileRequest::getTrackingId() const
{
    return mDataPrivate->getTrackingId();
}

std::string HttpDownloadToFileRequest::getRequestUri() const
{
    return mDataPrivate->getUri();
}

NetworkHttpHeaders HttpDownloadToFileRequest::getRequestHeaders() const
{
    return mDataPrivate->getHeaders();
}

int HttpDownloadToFileRequest::getTimeout() const
{
    return mDataPrivate->getTimeoutSecs();
}


std::string HttpDownloadToFileRequest::getDownloadFilePath() const
{
    return mDataPrivate->getDownloadFilePath();
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start HttpDownloadToFileRequest Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}