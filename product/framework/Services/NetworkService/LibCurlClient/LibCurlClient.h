#pragma once
#include "LibCurlClientExport.h"
namespace ucf::network::libcurl{
class LIBCURLCLIENT_EXPORT LibCurlClient
{
public:
    LibCurlClient();
    void makeHttpRequest();
};
}