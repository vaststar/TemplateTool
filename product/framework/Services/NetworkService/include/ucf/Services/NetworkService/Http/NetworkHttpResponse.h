#pragma once

#include <memory>
#include <ucf/Services/ServiceCommonFile/ServiceExport.h>

namespace ucf::network::http{
class SERVICE_EXPORT NetworkHttpResponse
{
public:
    NetworkHttpResponse();
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}