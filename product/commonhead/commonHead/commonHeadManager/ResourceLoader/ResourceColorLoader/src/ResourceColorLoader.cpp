#include "ResourceColorLoader.h"
#include <BuildWhiteThemeColorSet.h>
#include <ColorPaletteModel.h>

namespace commonHead {
std::shared_ptr<IResourceColorLoader> IResourceColorLoader::createInstance()
{
    return std::make_shared<ResourceColorLoader>();
}

ResourceColorLoader::ResourceColorLoader()
{
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
            }
        }
    }
    return model::Color{255, 255, 255, 255}; // Return white color as a default example.
}

void ResourceColorLoader::buildThemeColorSets()
{
    mThemeColorSets[model::ColorThemeType::White] = model::buildWhiteThemeColorSet();
    // Add other theme color sets as needed.
}
} // namespace commonHead