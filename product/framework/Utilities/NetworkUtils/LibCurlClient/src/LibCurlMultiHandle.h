#pragma once

#include <memory>

namespace ucf::utilities::network::libcurl{
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
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}