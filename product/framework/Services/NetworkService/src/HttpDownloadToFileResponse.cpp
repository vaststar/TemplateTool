#include <ucf/Services/NetworkService/Model/HttpDownloadToFileResponse.h>

namespace ucf::service::network::http{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class HttpDownloadToFileResponse::DataPrivate
{
public:
    DataPrivate();
    void setHttpResponseCode(int statusCode){ mResponseCode = statusCode;}
    int getHttpResponseCode() const{ return mResponseCode;}

    void setResponseHeaders(const NetworkHttpHeaders& headers){ mResponseHeaders = headers;}
    NetworkHttpHeaders getResponseHeaders() const{ return mResponseHeaders;}

    void setErrorData(const ResponseErrorStruct& errorData){ mErrorData = errorData;}
    std::optional<ResponseErrorStruct> getErrorData() const{ return mErrorData;}

    size_t getTotalSize(){ return mTotalSize;}
    void setTotalSize(size_t size){ mTotalSize = size;}
    
    bool isFinished() const{ return mIsFinished;}
    void setFinished(){ mIsFinished = true;}

    size_t getCurrentSize() const{ return mCurrentSize;}
    void addCurrentSize( size_t _size) { mCurrentSize += _size;}
private:
    int mResponseCode;
    NetworkHttpHeaders mResponseHeaders;
    std::optional<ResponseErrorStruct> mErrorData;
    size_t mTotalSize;
    size_t mCurrentSize;
    bool mIsFinished;
};

HttpDownloadToFileResponse::DataPrivate::DataPrivate()
    : mResponseCode(0)
    , mTotalSize(0)
    , mIsFinished(false)
    , mCurrentSize(0)
{

}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start HttpDownloadToFileResponse Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
HttpDownloadToFileResponse::HttpDownloadToFileResponse()
    : mDataPrivate(std::make_unique<HttpDownloadToFileResponse::DataPrivate>())
{

}

HttpDownloadToFileResponse::~HttpDownloadToFileResponse()
{

}


void HttpDownloadToFileResponse::setHttpResponseCode(int statusCode)
{
    mDataPrivate->setHttpResponseCode(statusCode);
}

int HttpDownloadToFileResponse::getHttpResponseCode() const
{
    return mDataPrivate->getHttpResponseCode();
}

void HttpDownloadToFileResponse::setResponseHeaders(const NetworkHttpHeaders& headers)
{
    mDataPrivate->setResponseHeaders(headers);
    auto item = std::find_if(headers.cbegin(), headers.cend(), [](const auto& headerKeyVal){
        return headerKeyVal.first == "Content-Length";
    });
    if (item != headers.cend())
    {
        mDataPrivate->setTotalSize(static_cast<size_t>(std::stoull(item->second)));
    }
}

NetworkHttpHeaders HttpDownloadToFileResponse::getResponseHeaders() const
{
    return mDataPrivate->getResponseHeaders();
}

void HttpDownloadToFileResponse::setErrorData(const ResponseErrorStruct& errorData)
{
    mDataPrivate->setErrorData(errorData);
}

std::optional<ResponseErrorStruct> HttpDownloadToFileResponse::getErrorData() const
{
    return mDataPrivate->getErrorData();
}

void HttpDownloadToFileResponse::appendResponseBody(const ByteBuffer& body)
{
    mDataPrivate->addCurrentSize(body.size());
}

size_t HttpDownloadToFileResponse::getCurrentSize() const
{
    return mDataPrivate->getCurrentSize();
}

size_t HttpDownloadToFileResponse::getTotalSize() const
{
    return mDataPrivate->getTotalSize();
}

bool HttpDownloadToFileResponse::isFinished() const
{
    return mDataPrivate->isFinished();
}

void HttpDownloadToFileResponse::setFinished()
{
    mDataPrivate->setFinished();
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start HttpDownloadToFileResponse Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}