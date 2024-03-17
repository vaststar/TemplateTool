#pragma once
#include "CoreFrameworkExport.h"
#include <string>
class COREFRAMEWORK_EXPORT ICoreFramework
{
public:
    virtual std::string getName() = 0;
};