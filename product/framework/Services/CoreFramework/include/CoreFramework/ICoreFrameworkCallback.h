#pragma once

#include "ServiceCommonFile/ServiceExport.h"

class SERVICE_EXPORT ICoreFrameworkCallback
{
public:
    virtual ~ICoreFrameworkCallback() = default;
    virtual void OnDataBaseInitialized() = 0;
    virtual void OnServiceInitialized() = 0;
};