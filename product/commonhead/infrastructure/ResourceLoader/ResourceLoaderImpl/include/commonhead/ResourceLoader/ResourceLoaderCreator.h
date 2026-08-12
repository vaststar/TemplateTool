#pragma once

#include <memory>

#include <commonhead/ResourceLoader/IResourceLoader.h>
#include <commonhead/ResourceLoader/ResourceLoaderImplExport.h>

namespace ucf::framework {
class ICoreFramework;
}

namespace commonHead::impl {

RESOURCE_LOADER_IMPL_API std::shared_ptr<IResourceLoader> createResourceLoader(
    std::weak_ptr<ucf::framework::ICoreFramework> coreFramework);

} // namespace commonHead::impl
