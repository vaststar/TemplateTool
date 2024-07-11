#pragma once

#include <memory>

namespace ucf::utilities::network::libcurl{
class LibCurlEasyHandle;
class LibCurlMultiHandleManager final
{
public:
    LibCurlMultiHandleManager();
    ~LibCurlMultiHandleManager();
    LibCurlMultiHandleManager(const LibCurlMultiHandleManager&) = delete;
    LibCurlMultiHandleManager(LibCurlMultiHandleManager&&) = delete;
    LibCurlMultiHandleManager& operator=(const LibCurlMultiHandleManager&) = delete;
    LibCurlMultiHandleManager& operator=(LibCurlMultiHandleManager&&) = delete;
public:
    void runLoop();
    void stopLoop();
    void insert(std::shared_ptr<LibCurlEasyHandle> request);
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}