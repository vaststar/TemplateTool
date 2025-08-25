#pragma once

#include <memory>
#include <commonHead/ResourceStringLoader/IResourceStringLoader.h>
#include <UIResourceStringLoader/UIResourceStringLoaderExport.h>

namespace UIResouce{
class UIResourceStringLoader_EXPORT UIResourceStringLoader final
{
public:
    static std::unique_ptr<commonHead::IResourceStringLoader> generateResourceStringLoader();
};
}
