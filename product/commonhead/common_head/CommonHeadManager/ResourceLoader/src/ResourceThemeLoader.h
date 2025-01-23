#pragma once

#include <mutex>
#include <memory>
#include <vector>

namespace ucf::service::model{
    enum class ThemeType;
}

namespace commonHead{
class Theme;

namespace model{
    enum class FontFamily;
    enum class FontSize;
    enum class FontWeight;
    enum class ColorItem;
    enum class ColorItemState;
    struct Font;
    struct Color;
}

class ResourceThemeLoader
{
public:
    ResourceThemeLoader();
    model::Font getFont(ucf::service::model::ThemeType themeType, model::FontFamily family, model::FontSize size, model::FontWeight weight, bool isItalic) const;
    model::Color getColor(ucf::service::model::ThemeType themeType, model::ColorItem colorItem, model::ColorItemState state) const;
private:
    std::shared_ptr<Theme> getOrCreateTheme(ucf::service::model::ThemeType themeType) const;
    std::shared_ptr<Theme> buildTheme(ucf::service::model::ThemeType themeType) const;
private:
    mutable std::mutex mThemeMutex;
    mutable std::vector<std::shared_ptr<Theme>> mThemes;
};
}