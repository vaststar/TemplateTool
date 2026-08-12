#pragma once

#include <memory>

#include <commonHead/ResourceFontLoader/IResourceFontLoader.h>
#include <commonHead/ResourceFontLoader/ResourceFontLoaderImplExport.h>

namespace commonHead::impl {

RESOURCE_FONT_LOADER_IMPL_API std::unique_ptr<IResourceFontLoader> createResourceFontLoader();

} // namespace commonHead::impl
