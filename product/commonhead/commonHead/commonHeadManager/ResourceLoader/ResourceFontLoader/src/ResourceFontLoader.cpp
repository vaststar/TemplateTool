#include "ResourceFontLoader.h"

#include <ucf/Utilities/OSUtils/OSUtils.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <BuildNormalThemeFontSet.h>

namespace commonHead{
std::unique_ptr<IResourceFontLoader> IResourceFontLoader::createInstance()
{
    return std::make_unique<ResourceFontLoader>();
}

ResourceFontLoader::ResourceFontLoader()
{
    COMMONHEAD_LOG_DEBUG("ResourceFontLoader created");
    buildThemeFontSets();
}

model::Font ResourceFontLoader::getFont(model::FontToken fontToken, model::FontThemeType theme) const
{
    if (auto it = mThemeFontSets.find(theme); it != mThemeFontSets.end())
    {
        const auto& themeFontSet = it->second;
        if (auto fontSetIt = themeFontSet.fontSets.find(fontToken); fontSetIt != themeFontSet.fontSets.end())
        {
            const auto& fontSet = fontSetIt->second;
            model::FontFamilys familys = model::getFontFamilys(fontSet.fontType);
            model::Font font;
            font.fontSize = model::getFontSize(fontSet.fontSize);
            font.fontWeight = model::getFontWeight(fontSet.fontWeight);
            font.isItalic = fontSet.isItalic;

            switch (ucf::utilities::OSUtils::getOSType())
            {
            case ucf::utilities::OSType::WINDOWS:
                if (!familys.windows.empty())
                {
                    font.fontFamily = familys.windows.front();
                }
                else if (!familys.default_.empty())
                {
                    font.fontFamily = familys.default_.front();
                }
                else
                {
                    COMMONHEAD_LOG_WARN("No font family found for Windows, using default font family.");
                }
                break;
            case ucf::utilities::OSType::MACOS:
            case ucf::utilities::OSType::APPLE_VISION:
                if (!familys.macosx.empty())
                {
                    font.fontFamily = familys.macosx.front();
                }
                else if (!familys.default_.empty())
                {
                    font.fontFamily = familys.default_.front();
                }
                else
                {
                    COMMONHEAD_LOG_WARN("No font family found for macOS, using default font family.");
                }
                break;
            case ucf::utilities::OSType::IOS:
                if (!familys.ios.empty())
                {
                    font.fontFamily = familys.ios.front();
                }
                else if (!familys.default_.empty())
                {
                    font.fontFamily = familys.default_.front();
                }
                else
                {
                    COMMONHEAD_LOG_WARN("No font family found for iOS, using default font family.");
                }
                break;
            case ucf::utilities::OSType::LINUX:
                if (!familys.linux.empty())
                {
                    font.fontFamily = familys.linux.front();
                }
                else if (!familys.default_.empty())
                {
                    font.fontFamily = familys.default_.front();
                }
                else
                {
                    COMMONHEAD_LOG_WARN("No font family found for Linux, using default font family.");
                }
                break;
            case ucf::utilities::OSType::ANDROID:
                if (!familys.android.empty())
                {
                    font.fontFamily = familys.android.front();
                }
                else if (!familys.default_.empty())
                {
                    font.fontFamily = familys.default_.front();
                }
                else
                {
                    COMMONHEAD_LOG_WARN("No font family found for Android, using default font family.");
                }
                break;
            default:
                COMMONHEAD_LOG_WARN("Unsupported OS type for font loading, OSType: " << ucf::utilities::OSUtils::getOSTypeName());
                if (!familys.default_.empty())
                {
                    font.fontFamily = familys.default_.front();
                }
                else
                {
                    COMMONHEAD_LOG_WARN("No default font family found, font family will be empty.");
                }
                break;
            }
            return font;
        }
        else
        {
            COMMONHEAD_LOG_WARN("unrecognized token: " << static_cast<int>(fontToken));
        }
    }
    COMMONHEAD_LOG_WARN("cant find font, token: " << static_cast<int>(fontToken) << ", theme: " << static_cast<int>(theme));
    return model::Font();
}

void ResourceFontLoader::buildThemeFontSets()
{
    mThemeFontSets[model::FontThemeType::Normal] = model::buildNormalThemeFontSet();
    // mThemeFontSets[model::FontThemeType::Large] = model::buildLargeThemeFontSet();
}

} // namespace commonHead