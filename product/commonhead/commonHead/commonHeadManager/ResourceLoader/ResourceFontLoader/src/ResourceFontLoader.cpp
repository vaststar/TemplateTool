#include "ResourceFontLoader.h"

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>

namespace commonHead{
std::unique_ptr<IResourceFontLoader> IResourceFontLoader::createInstance()
{
    return std::make_unique<ResourceFontLoader>();
}

ResourceFontLoader::ResourceFontLoader()
    : mFontSet(std::make_unique<FontSet>())
{
    COMMONHEAD_LOG_DEBUG("ResourceFontLoader created");
    initFonts();
}

void ResourceFontLoader::initFonts()
{
    COMMONHEAD_LOG_DEBUG("Initializing fonts");
    std::vector<std::shared_ptr<Fonts>> uiFonts;
    uiFonts.emplace_back(std::make_shared<Fonts>(model::FontFamily::SegoeUI));
    uiFonts.emplace_back(std::make_shared<Fonts>(model::FontFamily::Consolas));
    uiFonts.emplace_back(std::make_shared<Fonts>(model::FontFamily::SegoeUIEmoji));
    mFontSet->initFonts(uiFonts);
}

model::Font ResourceFontLoader::getFont(model::FontFamily family, model::FontSize size, model::FontWeight weight, bool isItalic) const
{
    if (auto fonts = mFontSet->getFonts(family))
    {
        return fonts->getFont(size, weight, isItalic);
    }
    COMMONHEAD_LOG_WARN("cant find font, family: " << static_cast<int>(family) << ", size: " << static_cast<int>(size) << ", weight: " << static_cast<int>(weight) << ", isItalic: " << isItalic);
    return model::Font();
}

} // namespace commonHead