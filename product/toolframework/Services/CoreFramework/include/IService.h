#pragma once

#include "ServiceExport.h"

#include <string>

class SERVICE_EXPORT IService{
public:
    virtual std::string getServiceName() const = 0;
};