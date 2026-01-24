#pragma once

#include <memory>

namespace ucf::utilities::network::libcurl{

/// @brief Ensures curl is globally initialized (thread-safe, call before any curl operations)
void ensureCurlGlobalInit();

class LibCurlEasyHandle;
class LibCurlMultiHandle final
{
public:
    LibCurlMultiHandle();
    ~LibCurlMultiHandle();
    LibCurlMultiHandle(const LibCurlMultiHandle&) = delete;
    LibCurlMultiHandle(LibCurlMultiHandle&&) = delete;
    LibCurlMultiHandle& operator=(const LibCurlMultiHandle&) = delete;
    LibCurlMultiHandle& operator=(LibCurlMultiHandle&&) = delete;

public:
    int addEasyHandle(std::shared_ptr<LibCurlEasyHandle> easyHandle);
    int removeEasyHandle(std::shared_ptr<LibCurlEasyHandle> easyHandle);

    void performRequests();
    void stop();
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}