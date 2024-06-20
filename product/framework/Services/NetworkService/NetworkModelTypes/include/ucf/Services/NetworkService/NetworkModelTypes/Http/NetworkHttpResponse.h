#pragma once

#include <memory>
#include <string>
#include <ucf/Services/NetworkService/NetworkModelTypes/NetworkModelTypesExport.h>

namespace ucf::network::http{
struct NETWORKTYPE_EXPORT HttpResponseMetrics{
    int requestStartTime{ 0 };
    int dnsLookupMs{ 0 };
    int connectMs{ 0 };
    int sendRequestMs{ 0 };
    int receiveResponseMs{ 0 };
    size_t responseLength{ 0 };
    std::string httpVersion;
};
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