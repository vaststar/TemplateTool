#include <vector>
#include <mutex>

#include <curl/curl.h>

#include "LibCurlMultiHandle.h"
#include "LibCurlClientLogger.h"
#include "LibCurlEasyHandle.h"

namespace ucf::service::network::libcurl{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class LibCurlMultiHandle::DataPrivate
{
public:
    DataPrivate();
    ~DataPrivate();
    int addEasyHandle(std::shared_ptr<LibCurlEasyHandle> easyHandle);
    int removeEasyHandle(std::shared_ptr<LibCurlEasyHandle> easyHandle);
    int perform(int* count);
    int poll(int timeout_ms);
    void completeCurrentRequest();
private:
    CURLMsg* read();
    std::shared_ptr<LibCurlEasyHandle> getEasyHanleFromCURL(CURL* handle);
private:
    std::mutex mCURLAccess;
    CURLM* mHandle;

    std::mutex mRequestsAccess;
    std::vector<std::shared_ptr<LibCurlEasyHandle>> mRequests;
};

LibCurlMultiHandle::DataPrivate::DataPrivate()
    : mHandle(curl_multi_init())
{
}

LibCurlMultiHandle::DataPrivate::~DataPrivate()
{
    if (mHandle)
    {
        curl_multi_cleanup(mHandle);
    }
}

int LibCurlMultiHandle::DataPrivate::addEasyHandle(std::shared_ptr<LibCurlEasyHandle> easyHandle)
{
    {
        std::scoped_lock lo(mRequestsAccess);
        mRequests.push_back(easyHandle);
    }
    
    std::scoped_lock lo(mCURLAccess);
    auto code = curl_multi_add_handle(mHandle, easyHandle->getHandle());
    if (CURLM_OK != code)
    {
        LIBCURL_LOG_INFO("add easy handle error: " << curl_multi_strerror(code));
    }
    LIBCURL_LOG_DEBUG("add easy handle");
    return code;
}

int LibCurlMultiHandle::DataPrivate::removeEasyHandle(std::shared_ptr<LibCurlEasyHandle> easyHandle)
{
    {
        std::scoped_lock lo(mRequestsAccess);
        std::erase(mRequests, easyHandle);
    }

    std::scoped_lock lo(mCURLAccess);
    auto code = curl_multi_remove_handle(mHandle, easyHandle->getHandle());
    if (CURLM_OK != code)
    {
        LIBCURL_LOG_INFO("remove easy handle error: " << curl_multi_strerror(code));
    }
    LIBCURL_LOG_DEBUG("remove easy handle");
    return code;
}

int LibCurlMultiHandle::DataPrivate::perform(int* count)
{
    std::scoped_lock lo(mCURLAccess);
    auto code = curl_multi_perform(mHandle, count);
    if (CURLM_OK != code)
    {
        LIBCURL_LOG_INFO("multi handle perform error: " << curl_multi_strerror(code));
    }
    return code;
}

int LibCurlMultiHandle::DataPrivate::poll(int timeout_ms)
{
    auto code = curl_multi_poll(mHandle, nullptr, 0, timeout_ms, nullptr);
    if (CURLM_OK != code)
    {
        LIBCURL_LOG_INFO("multi handle perform error: " << curl_multi_strerror(code));
    }
    return code;
}

CURLMsg* LibCurlMultiHandle::DataPrivate::read()
{
    std::scoped_lock lo(mCURLAccess);
    int count = -1;
    return curl_multi_info_read(mHandle, &count);
}

std::shared_ptr<LibCurlEasyHandle> LibCurlMultiHandle::DataPrivate::getEasyHanleFromCURL(CURL* handle)
{
    std::scoped_lock lo(mRequestsAccess);
    auto iter = std::find_if(mRequests.cbegin(), mRequests.cend(), [handle](const auto& request){
        return request->getHandle() == handle;
    });

    if (iter != mRequests.end())
    {
        return *iter;
    }
    return nullptr;
}

void LibCurlMultiHandle::DataPrivate::completeCurrentRequest()
{
    CURLMsg* msg = nullptr;
    while((msg = read()))
    {
        if (CURLMSG_DONE == msg->msg)
        {
            if (auto libCurlHandle = getEasyHanleFromCURL(msg->easy_handle))
            {
                libCurlHandle->finishHandle(msg->data.result);
                removeEasyHandle(libCurlHandle);
            }
        }
        else
        {
            LIBCURL_LOG_WARN("Invalid message's code: " << msg->msg);
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic/////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start LibCurlMultiHandle Logic////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

LibCurlMultiHandle::LibCurlMultiHandle()
    : mDataPrivate(std::make_unique<DataPrivate>())
{
    LIBCURL_LOG_DEBUG("create multi handle");
}

LibCurlMultiHandle::~LibCurlMultiHandle()
{
    LIBCURL_LOG_DEBUG("destory multi handle");
}

int LibCurlMultiHandle::addEasyHandle(std::shared_ptr<LibCurlEasyHandle> easyHandle)
{
    return mDataPrivate->addEasyHandle(easyHandle);
}

int LibCurlMultiHandle::removeEasyHandle(std::shared_ptr<LibCurlEasyHandle> easyHandle)
{
    return mDataPrivate->removeEasyHandle(easyHandle);
}

void LibCurlMultiHandle::performRequests()
{
    int runningHandles = 0;
    do
    {
        if (auto code = mDataPrivate->perform(&runningHandles); CURLM_OK !=  code)
        {
            LIBCURL_LOG_WARN("multi perform failed:" << code);
            break;
        }

        if (runningHandles > 0)
        {
            if (auto code = mDataPrivate->poll(100); CURLM_OK !=  code)
            {
                LIBCURL_LOG_WARN("multi poll failed:" << code);
                break;
            }
        }
    }while(runningHandles > 0);
    mDataPrivate->completeCurrentRequest();
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start LibCurlMultiHandle Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}