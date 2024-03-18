#pragma once
#include "ServiceExport.h"
#include <string>
#include <memory>
#include <vector>

class IService;

class SERVICE_EXPORT ICoreFramework
{
public:
    virtual std::string getName() const = 0;
    virtual std::vector<std::weak_ptr<IService>> getServices() const = 0;
};