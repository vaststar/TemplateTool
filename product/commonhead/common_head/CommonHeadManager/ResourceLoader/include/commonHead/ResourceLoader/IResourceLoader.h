#pragma once

#include <vector>
#include <memory>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

namespace commonHead{
class COMMONHEAD_EXPORT IResourceLoader
{
public:
    ~IResourceLoader() = default;
public:
    static std::shared_ptr<IResourceLoader> createInstance();
};
}