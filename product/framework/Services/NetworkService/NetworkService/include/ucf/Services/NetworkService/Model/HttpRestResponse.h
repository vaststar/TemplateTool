#pragma once

#include <string>
#include <memory>
#include <functional>

#include <ucf/Services/ServiceCommonFile/ServiceExport.h>

namespace ucf::service::network::http{
class SERVICE_EXPORT HttpRestResponse final
{
public:
    HttpRestResponse();
    ~HttpRestResponse();
private:private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};

}