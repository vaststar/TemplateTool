#pragma once

#include <memory>
#include <ucf/NetworkService/NetworkModelTypes/NetworkModelTypesExport.h>

namespace ucf::network::http{
class NETWORKTYPE_EXPORT NetworkHttpResponse final
{
public:
    NetworkHttpResponse();
    ~NetworkHttpResponse();
    NetworkHttpResponse(const NetworkHttpResponse&) = delete;
    NetworkHttpResponse(NetworkHttpResponse&&) = delete;
    NetworkHttpResponse& operator=(const NetworkHttpResponse&) = delete;
    NetworkHttpResponse& operator=(NetworkHttpResponse&&) = delete;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}