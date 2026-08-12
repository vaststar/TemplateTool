#pragma once

#include <memory>

#include <commonhead/ResourceAssetLoader/IResourceAssetLoader.h>
#include <commonhead/ResourceAssetLoader/ResourceAssetLoaderImplExport.h>

namespace commonHead::impl {

RESOURCE_ASSET_LOADER_IMPL_API std::unique_ptr<IResourceAssetLoader> createResourceAssetLoader();

} // namespace commonHead::impl
