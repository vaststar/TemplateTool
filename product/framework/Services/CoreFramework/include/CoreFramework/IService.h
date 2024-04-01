#pragma once

#include "ServiceCommonFile/ServiceExport.h"

#include <string>

class SERVICE_EXPORT IService{
public:
    virtual std::string getServiceName() const = 0;
    virtual ~IService() = default;
    virtual void initService() = 0;
};