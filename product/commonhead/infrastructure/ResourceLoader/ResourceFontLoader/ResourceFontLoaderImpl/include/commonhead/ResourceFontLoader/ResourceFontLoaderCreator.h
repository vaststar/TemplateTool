#pragma once

#include <memory>

#include <commonhead/ResourceFontLoader/IResourceFontLoader.h>
#include <commonhead/ResourceFontLoader/ResourceFontLoaderImplExport.h>

namespace commonHead::impl {

RESOURCE_FONT_LOADER_IMPL_API std::unique_ptr<IResourceFontLoader> createResourceFontLoader();

} // namespace commonHead::impl
