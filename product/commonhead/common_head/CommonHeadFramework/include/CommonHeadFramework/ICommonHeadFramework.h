#pragma once

#include <string>
#include <memory>
#include "CommonHeadCommonFile/CommonHeadExport.h"

class COMMONHEAD_EXPORT ICommonHeadFramework
{
public:
    virtual std::string getName() const = 0;
public:
    static std::shared_ptr<ICommonHeadFramework> CreateInstance();
};