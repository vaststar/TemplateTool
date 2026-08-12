#pragma once

#include <memory>

#include <commonhead/ResourceColorLoader/IResourceColorLoader.h>
#include <commonhead/ResourceColorLoader/ResourceColorLoaderImplExport.h>

namespace commonHead::impl {

RESOURCE_COLOR_LOADER_IMPL_API std::unique_ptr<IResourceColorLoader> createResourceColorLoader();

} // namespace commonHead::impl
