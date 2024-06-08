#pragma once

#include <memory>
#include <ucf/Services/ServiceCommonFile/ServiceExport.h>

namespace ucf::network::http{
class SERVICE_EXPORT NetworkHttpRequest
{
public:
    NetworkHttpRequest();
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}