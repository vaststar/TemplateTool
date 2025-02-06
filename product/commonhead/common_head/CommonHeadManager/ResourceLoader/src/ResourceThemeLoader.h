#pragma once

#include <mutex>
#include <memory>
#include <vector>

namespace ucf::service::model{
    enum class ThemeType;
}

namespace commonHead{
class ResourceTheme;

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
    void addTheme(ucf::service::model::ThemeType themeType);
    model::Font getFont(ucf::service::model::ThemeType themeType, model::FontFamily family, model::FontSize size, model::FontWeight weight, bool isItalic) const;
    model::Color getColor(ucf::service::model::ThemeType themeType, model::ColorItem colorItem, model::ColorItemState state) const;

private:
    std::shared_ptr<ResourceTheme> buildTheme(ucf::service::model::ThemeType themeType) const;
    std::shared_ptr<ResourceTheme> getTheme(ucf::service::model::ThemeType themeType) const;
private:
    mutable std::mutex mThemeMutex;
    std::vector<std::shared_ptr<ResourceTheme>> mThemes;
};
}