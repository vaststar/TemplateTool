#include "ResourceFontLoader.h"

#include <BuildNormalThemeFontSet.h>
#include <commonHead/ResourceFontLoader/ResourceFontLoaderCreator.h>
#include <ucf/utilities/OSUtils/OSUtils.h>

#include "LoggerDefine.h"

namespace commonHead {

namespace impl {

std::unique_ptr<IResourceFontLoader> createResourceFontLoader()
{
    return std::make_unique<ResourceFontLoader>();
}

} // namespace impl

ResourceFontLoader::ResourceFontLoader()
{
    RESOURCE_FONT_LOADER_LOG_DEBUG("ResourceFontLoader created");
    buildThemeFontSets();
}

model::Font ResourceFontLoader::getFont(
    model::FontToken fontToken,
    model::FontThemeType theme) const
{
    if (auto it = mThemeFontSets.find(theme); it != mThemeFontSets.end())
    {
        const auto& themeFontSet = it->second;
        if (auto fontSetIt = themeFontSet.fontSets.find(fontToken); fontSetIt != themeFontSet.fontSets.end())
        {
            const auto& fontSet = fontSetIt->second;
            const model::FontFamilys familys = model::getFontFamilys(fontSet.fontType);
            model::Font font;
            font.fontSize = model::getFontSize(fontSet.fontSize);
            font.fontWeight = model::getFontWeight(fontSet.fontWeight);
            font.isItalic = fontSet.isItalic;

            switch (ucf::utilities::OSUtils::getOSType())
            {
                case ucf::utilities::OSType::WINDOWS:
                    if (!familys.windows.empty())
                    {
                        font.fontFamilies = familys.windows;
                    }
                    else if (!familys.default_.empty())
                    {
                        font.fontFamilies = familys.default_;
                    }
                    else
                    {
                        RESOURCE_FONT_LOADER_LOG_WARN("No font family found for Windows, using default font family.");
                    }
                    break;
                case ucf::utilities::OSType::MACOS:
                case ucf::utilities::OSType::APPLE_VISION:
                    if (!familys.macosx.empty())
                    {
                        font.fontFamilies = familys.macosx;
                    }
                    else if (!familys.default_.empty())
                    {
                        font.fontFamilies = familys.default_;
                    }
                    else
                    {
                        RESOURCE_FONT_LOADER_LOG_WARN("No font family found for macOS, using default font family.");
                    }
                    break;
                case ucf::utilities::OSType::IOS:
                    if (!familys.ios.empty())
                    {
                        font.fontFamilies = familys.ios;
                    }
                    else if (!familys.default_.empty())
                    {
                        font.fontFamilies = familys.default_;
                    }
                    else
                    {
                        RESOURCE_FONT_LOADER_LOG_WARN("No font family found for iOS, using default font family.");
                    }
                    break;
                case ucf::utilities::OSType::LINUX:
                    if (!familys.linux.empty())
                    {
                        font.fontFamilies = familys.linux;
                    }
                    else if (!familys.default_.empty())
                    {
                        font.fontFamilies = familys.default_;
                    }
                    else
                    {
                        RESOURCE_FONT_LOADER_LOG_WARN("No font family found for Linux, using default font family.");
                    }
                    break;
                case ucf::utilities::OSType::ANDROID:
                    if (!familys.android.empty())
                    {
                        font.fontFamilies = familys.android;
                    }
                    else if (!familys.default_.empty())
                    {
                        font.fontFamilies = familys.default_;
                    }
                    else
                    {
                        RESOURCE_FONT_LOADER_LOG_WARN("No font family found for Android, using default font family.");
                    }
                    break;
                default:
                    RESOURCE_FONT_LOADER_LOG_WARN(
                        "Unsupported OS type for font loading, OSType: "
                        << ucf::utilities::OSUtils::getOSTypeName());
                    if (!familys.default_.empty())
                    {
                        font.fontFamilies = familys.default_;
                    }
                    else
                    {
                        RESOURCE_FONT_LOADER_LOG_WARN("No default font family found, font family will be empty.");
                    }
                    break;
            }
            return font;
        }
        RESOURCE_FONT_LOADER_LOG_WARN("unrecognized token: " << static_cast<int>(fontToken));
    }
    RESOURCE_FONT_LOADER_LOG_WARN(
        "cant find font, token: " << static_cast<int>(fontToken)
        << ", theme: " << static_cast<int>(theme));
    return {};
}

void ResourceFontLoader::buildThemeFontSets()
{
    mThemeFontSets[model::FontThemeType::Normal] = model::buildNormalThemeFontSet();
}

} // namespace commonHead
