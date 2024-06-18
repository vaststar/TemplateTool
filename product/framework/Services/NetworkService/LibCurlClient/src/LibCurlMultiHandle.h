#pragma once

#include <mutex>
#include <curl/curl.h>

namespace ucf::network::libcurl{
class LibCurlMultiHandle
{
public:
    LibCurlMultiHandle();
    ~LibCurlMultiHandle();
    LibCurlMultiHandle(const LibCurlMultiHandle&) = delete;
    LibCurlMultiHandle(LibCurlMultiHandle&&) = delete;
    LibCurlMultiHandle& operator=(const LibCurlMultiHandle&) = delete;
    LibCurlMultiHandle& operator=(LibCurlMultiHandle&&) = delete;

    int addEasyHandle(CURL* easyHandle);
    int removeEasyHandle(CURL* easyHandle);

    int perform(int* count);
    int poll(curl_waitfd extraFds[], unsigned int fdCount, int timeoutMS, int* numfds);
    CURLMsg* read();
private:
    std::mutex mDataAccess;
    CURLM* mHandle;
};
}