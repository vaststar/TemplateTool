#include <ucf/Services/NetworkService/NetworkModelTypes/Http/NetworkHttpResponse.h>


#include "NetworkHttpCallbackHandler.h"

namespace ucf::service::network::http{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class NetworkHttpCallbackHandler::DataPrivate{
public:
    DataPrivate();
    void setResponseCallback(const NetworkHttpResponseCallbackFunc& callbackFunc){mResponseCallBack = callbackFunc;}
    NetworkHttpResponseCallbackFunc getResponseCallback() const{return mResponseCallBack;}

    NetworkHttpResponse& getResponse(){return mResponse;}

    void appendBuffer(const ByteBuffer& buffer){mCachedBuffer.insert(mCachedBuffer.end(), buffer.begin(), buffer.end());}
    const ByteBuffer& getCachedBuffer() const{return mCachedBuffer;}
private:
    NetworkHttpResponseCallbackFunc mResponseCallBack;
    NetworkHttpResponse mResponse;
    ByteBuffer mCachedBuffer;
};

NetworkHttpCallbackHandler::DataPrivate::DataPrivate()
{

}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start NetworkHttpCallbackHandler Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
NetworkHttpCallbackHandler::NetworkHttpCallbackHandler()
    : mDataPrivate(std::make_unique<NetworkHttpCallbackHandler::DataPrivate>())
{

}

NetworkHttpCallbackHandler::~NetworkHttpCallbackHandler()
{

}

void NetworkHttpCallbackHandler::setResponseCallback(const NetworkHttpResponseCallbackFunc& callbackFunc)
{
    mDataPrivate->setResponseCallback(callbackFunc);
}

void NetworkHttpCallbackHandler::setResponseHeader(int statusCode, const NetworkHttpHeaders& headers, std::optional<ResponseErrorStruct> errorData)
{
    mDataPrivate->getResponse().setHttpResponseCode(statusCode);
    mDataPrivate->getResponse().setResponseHeaders(headers);
    if (errorData)
    {
        mDataPrivate->getResponse().setErrorData(*errorData);
    }
}

void NetworkHttpCallbackHandler::appendResponseBody(const ByteBuffer& buffer, bool isFinished)
{
    mDataPrivate->appendBuffer(buffer);
    if (isFinished)
    {
        std::string bodyString = std::string(mDataPrivate->getCachedBuffer().begin(), mDataPrivate->getCachedBuffer().end());
        mDataPrivate->getResponse().setResponseBody(std::move(bodyString));
    }
}

void NetworkHttpCallbackHandler::completeResponse(const HttpResponseMetrics& metrics)
{
    if (mDataPrivate->getResponseCallback())
    {
        mDataPrivate->getResponseCallback()(mDataPrivate->getResponse());
    }
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish NetworkHttpCallbackHandler Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}