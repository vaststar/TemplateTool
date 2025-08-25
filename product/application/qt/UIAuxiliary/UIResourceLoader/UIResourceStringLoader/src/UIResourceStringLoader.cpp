#include <UIResourceStringLoader/UIResourceStringLoader.h>

#include "AppUIStringLoader.h"
namespace UIResouce{
std::unique_ptr<commonHead::IResourceStringLoader> UIResourceStringLoader::generateResourceStringLoader()
{
    return std::make_unique<AppUIStringLoader>();
}
}