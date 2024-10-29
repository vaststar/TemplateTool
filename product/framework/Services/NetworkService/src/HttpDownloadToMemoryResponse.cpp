#include <ucf/Services/NetworkService/Model/HttpDownloadToMemoryResponse.h>

namespace ucf::service::network::http{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class HttpDownloadToMemoryResponse::DataPrivate
{
public:
    DataPrivate();
    void setHttpResponseCode(int statusCode){ mResponseCode = statusCode;}
    int getHttpResponseCode() const{ return mResponseCode;}

    void setResponseHeaders(const NetworkHttpHeaders& headers){ mResponseHeaders = headers;}
    NetworkHttpHeaders getResponseHeaders() const{ return mResponseHeaders;}

    void setErrorData(const ResponseErrorStruct& errorData){ mErrorData = errorData;}
    std::optional<ResponseErrorStruct> getErrorData() const{ return mErrorData;}

    void appendResponseBody(const ByteBuffer& buffer){ mResponseBody.insert(mResponseBody.end(), buffer.begin(), buffer.end());}
    void setResponseBody(const ByteBuffer& buffer){ mResponseBody = buffer;}
    const ByteBuffer& getResponseBody() const{ return mResponseBody;}

    size_t getTotalSize(){ return mTotalSize;}
    void setTotalSize(size_t size){ mTotalSize = size;}
    
    bool isFinished() const{ return mIsFinished;}
    void setFinished(){ mIsFinished = true;}
private:
    int mResponseCode;
    NetworkHttpHeaders mResponseHeaders;
    std::optional<ResponseErrorStruct> mErrorData;
    ByteBuffer mResponseBody;
    size_t mTotalSize;
    bool mIsFinished;
};

HttpDownloadToMemoryResponse::DataPrivate::DataPrivate()
    : mResponseCode(0)
    , mTotalSize(0)
    , mIsFinished(false)
{

}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start HttpDownloadToMemoryResponse Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
HttpDownloadToMemoryResponse::HttpDownloadToMemoryResponse()
    : mDataPrivate(std::make_unique<HttpDownloadToMemoryResponse::DataPrivate>())
{

}

HttpDownloadToMemoryResponse::~HttpDownloadToMemoryResponse()
{

}


void HttpDownloadToMemoryResponse::setHttpResponseCode(int statusCode)
{
    mDataPrivate->setHttpResponseCode(statusCode);
}

int HttpDownloadToMemoryResponse::getHttpResponseCode() const
{
    return mDataPrivate->getHttpResponseCode();
}

void HttpDownloadToMemoryResponse::setResponseHeaders(const NetworkHttpHeaders& headers)
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

NetworkHttpHeaders HttpDownloadToMemoryResponse::getResponseHeaders() const
{
    return mDataPrivate->getResponseHeaders();
}

void HttpDownloadToMemoryResponse::setErrorData(const ResponseErrorStruct& errorData)
{
    mDataPrivate->setErrorData(errorData);
}

std::optional<ResponseErrorStruct> HttpDownloadToMemoryResponse::getErrorData() const
{
    return mDataPrivate->getErrorData();
}

void HttpDownloadToMemoryResponse::appendResponseBody(const ByteBuffer& buffer)
{
    mDataPrivate->appendResponseBody(buffer);
}

void HttpDownloadToMemoryResponse::setResponseBody(const ByteBuffer& buffer)
{
    mDataPrivate->setResponseBody(buffer);
}

const ByteBuffer& HttpDownloadToMemoryResponse::getResponseBody() const
{
    return mDataPrivate->getResponseBody();
}

size_t HttpDownloadToMemoryResponse::getTotalSize() const
{
    return mDataPrivate->getTotalSize();
}

bool HttpDownloadToMemoryResponse::isFinished() const
{
    return mDataPrivate->isFinished();
}

void HttpDownloadToMemoryResponse::setFinished()
{
    mDataPrivate->setFinished();
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start HttpDownloadToMemoryResponse Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}