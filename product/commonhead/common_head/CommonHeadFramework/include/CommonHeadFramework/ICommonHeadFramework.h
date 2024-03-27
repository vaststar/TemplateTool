#pragma once

#include <string>
#include <memory>
#include "CommonHeadCommonFile/CommonHeadExport.h"

class ICoreFramework;
using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
class COMMONHEAD_EXPORT ICommonHeadFramework
{
public:
    virtual std::string getName() const = 0;
    virtual ICoreFrameworkWPtr getCoreFramework() const = 0;
public:
    static std::shared_ptr<ICommonHeadFramework> CreateInstance(ICoreFrameworkWPtr coreframework);
};