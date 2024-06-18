
#include "LibCurlMultiHandle.h"
#include "LibCurlClientLogger.h"

namespace ucf::network::libcurl{
LibCurlMultiHandle::LibCurlMultiHandle()
    : mHandle(curl_multi_init())
{
    LIBCURL_LOG_DEBUG("create multi handle");
}

LibCurlMultiHandle::~LibCurlMultiHandle()
{
    if (mHandle)
    {
        curl_multi_cleanup(mHandle);
    }
}

int LibCurlMultiHandle::addEasyHandle(CURL* easyHandle)
{
    std::scoped_lock lo(mDataAccess);
    auto code = curl_multi_add_handle(mHandle, easyHandle);
    if (CURLM_OK != code)
    {
        LIBCURL_LOG_INFO("add easy handle error: " << curl_multi_strerror(code));
    }
    LIBCURL_LOG_DEBUG("add easy handle");
    return code;
}

int LibCurlMultiHandle::removeEasyHandle(CURL* easyHandle)
{
    std::scoped_lock lo(mDataAccess);
    auto code = curl_multi_remove_handle(mHandle, easyHandle);
    if (CURLM_OK != code)
    {
        LIBCURL_LOG_INFO("remove easy handle error: " << curl_multi_strerror(code));
    }
    LIBCURL_LOG_DEBUG("remove easy handle");
    return code;
}

int LibCurlMultiHandle::perform(int* count)
{
    std::scoped_lock lo(mDataAccess);
    auto code = curl_multi_perform(mHandle, count);
    if (CURLM_OK != code)
    {
        LIBCURL_LOG_INFO("multi handle perform error: " << curl_multi_strerror(code));
    }
    LIBCURL_LOG_DEBUG("");
    return code;
}

int LibCurlMultiHandle::poll(curl_waitfd extraFds[], unsigned int fdCount, int timeoutMS, int* numfds)
{
    auto code = curl_multi_poll(mHandle, extraFds, fdCount, timeoutMS, numfds);
    if (CURLM_OK != code)
    {
        LIBCURL_LOG_INFO("multi handle perform error: " << curl_multi_strerror(code));
    }
    LIBCURL_LOG_DEBUG("");
    return code;
}

CURLMsg* LibCurlMultiHandle::read()
{
    int count = -1;
    return curl_multi_info_read(mHandle, &count);
}
}