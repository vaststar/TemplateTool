#pragma once

#include <memory>
#include <string>

#include <curl/curl.h>

#include <ucf/Services/NetworkService/NetworkModelTypes/Http/NetworkHttpTypes.h>
namespace ucf::service::network::libcurl{
class PayloadData
{
public:
    virtual ~PayloadData() = default;
    virtual size_t readData(char* data, size_t size) = 0;
    virtual int seekData(curl_off_t offset, int origin) = 0;
};

class StringPayloadData final: public PayloadData{
public:
    explicit StringPayloadData(const std::string& strData);
    ~StringPayloadData();
    virtual size_t readData(char* data, size_t size) override;
    virtual int seekData(curl_off_t offset, int origin) override;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};


class BufferPayloadData final: public PayloadData{
public:
    BufferPayloadData(ucf::service::network::http::ByteBufferPtr data, ucf::service::network::http::UploadProgressFunction progressFunc);
    ~BufferPayloadData();
    virtual size_t readData(char* data, size_t size) override;
    virtual int seekData(curl_off_t offset, int origin) override;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};

class FilePayloadData final: public PayloadData{
public:
    FilePayloadData(const std::string& filePath, ucf::service::network::http::UploadProgressFunction progressFunc);
    ~FilePayloadData();
    virtual size_t readData(char* data, size_t size) override;
    virtual int seekData(curl_off_t offset, int origin) override;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}