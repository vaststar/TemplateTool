#pragma once

#include <memory>

#include <commonHead/ResourceColorLoader/IResourceColorLoader.h>
#include <commonHead/ResourceColorLoader/ResourceColorLoaderImplExport.h>

namespace commonHead::impl {

RESOURCE_COLOR_LOADER_IMPL_API std::unique_ptr<IResourceColorLoader> createResourceColorLoader();

} // namespace commonHead::impl
