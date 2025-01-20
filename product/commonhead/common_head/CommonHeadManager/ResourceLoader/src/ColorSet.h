#pragma once 

#include <vector>
#include <memory>
#include <commonHead/ResourceLoader/ResourceLoaderModel.h>

namespace commonHead{

class Colors
{
public:
    Colors(model::ColorItem colorEnum, const model::Color& normal);
    Colors(model::ColorItem colorEnum, const model::Color& normal, const model::Color& hovered, const model::Color& pressed, const model::Color& disabled, const model::Color& focused, const model::Color& checked);

    model::ColorItem getColorEnum() const;

    model::Color getColor(model::ColorItemState state) const;
private:
    model::ColorItem mColorEnum;
    model::Color mNormal;
    model::Color mHovered;
    model::Color mPressed;
    model::Color mDisabled;
    model::Color mFocused;
    model::Color mChecked;
};

class ColorSet
{
public:
    ColorSet();
    void initColors(const std::vector<std::shared_ptr<Colors>>& colors);
    std::shared_ptr<Colors> getColors(model::ColorItem) const;
private:
    std::vector<std::shared_ptr<Colors>> mColors;
};
}