#include "ResourceColorLoader.h"
#include <BuildLightThemeColorSet.h>
#include <BuildDarkThemeColorSet.h>
#include <ColorPaletteModel.h>
#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>

namespace commonHead {
std::unique_ptr<IResourceColorLoader> IResourceColorLoader::createInstance()
{
    return std::make_unique<ResourceColorLoader>();
}

ResourceColorLoader::ResourceColorLoader()
{
    COMMONHEAD_LOG_DEBUG("ResourceColorLoader created");
    buildThemeColorSets();
}

model::Color ResourceColorLoader::getColor(model::ColorToken token, model::ColorState state, model::ColorThemeType theme) const
{
    if (auto it = mThemeColorSets.find(theme); it != mThemeColorSets.end())
    {
        const auto& themeColorSet = it->second;
        if (auto colorSetIt = themeColorSet.colorSets.find(token); colorSetIt != themeColorSet.colorSets.end())
        {
            const auto& colorSet = colorSetIt->second;
            switch (state)
            {
                case model::ColorState::Normal:
                    return model::getColorPalette(colorSet.normal).toColor();
                case model::ColorState::Hovered:
                    return model::getColorPalette(colorSet.hovered).toColor();
                case model::ColorState::Pressed:
                    return model::getColorPalette(colorSet.pressed).toColor();
                case model::ColorState::Disabled:
                    return model::getColorPalette(colorSet.disabled).toColor();
                case model::ColorState::Focused:
                    return model::getColorPalette(colorSet.focused).toColor();
                case model::ColorState::Checked:
                    return model::getColorPalette(colorSet.checked).toColor();
                case model::ColorState::Selected:
                    return model::getColorPalette(colorSet.selected).toColor();
            }
        }
        else
        {
            COMMONHEAD_LOG_WARN("unrecognized token: " << static_cast<int>(token));
        }
    }
    return model::Color{255, 255, 255, 255}; // Return white color as a default example.
}

void ResourceColorLoader::buildThemeColorSets()
{
    mThemeColorSets[model::ColorThemeType::Light] = model::buildLightThemeColorSet();
    mThemeColorSets[model::ColorThemeType::Dark] = model::buildDarkThemeColorSet();
    // Add other theme color sets as needed.
}
} // namespace commonHead