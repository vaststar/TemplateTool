#include "ColorSet.h"

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
namespace commonHead{

Colors::Colors(model::ColorItem colorEnum, const model::Color& normal)
    : mColorEnum(colorEnum)
    , mNormal(normal)
    , mHovered(normal)
    , mPressed(normal)
    , mDisabled(normal)
    , mFocused(normal)
    , mChecked(normal)
{

}

Colors::Colors(model::ColorItem colorEnum, const model::Color& normal, const model::Color& hovered, const model::Color& pressed, const model::Color& disabled, const model::Color& focused, const model::Color& checked)
    : mColorEnum(colorEnum)
    , mNormal(normal)
    , mHovered(hovered)
    , mPressed(pressed)
    , mDisabled(disabled)
    , mFocused(focused)
    , mChecked(checked)
{

}

model::Color Colors::getColor(model::ColorItemState state) const
{ 
    switch (state)
    {
    case model::ColorItemState::ColorItemState_Normal:
        return mNormal;
    case model::ColorItemState::ColorItemState_Hovered:
        return mHovered;
    case model::ColorItemState::ColorItemState_Pressed:
        return mPressed;
    case model::ColorItemState::ColorItemState_Disabled:
        return mDisabled;
    case model::ColorItemState::ColorItemState_Focused:
        return mFocused;
    case model::ColorItemState::ColorItemState_Checked:
        return mChecked;
    default:
        return mNormal;
    }
}

model::ColorItem Colors::getColorEnum() const
{
    return mColorEnum;
}

ColorSet::ColorSet()
{

}

void ColorSet::initColors(const std::vector<std::shared_ptr<Colors>>& colors)
{
    mColors = colors;
}

std::shared_ptr<Colors> ColorSet::getColors(model::ColorItem colorEnum) const
{
    auto iter = std::find_if(mColors.cbegin(), mColors.cend(), [colorEnum](const auto uiColor){
        return colorEnum == uiColor->getColorEnum();
    });
    if (iter != mColors.cend())
    {
        return (*iter);
    }
    COMMONHEAD_LOG_DEBUG("can't find this ui enum: " << static_cast<int>(colorEnum));
    return nullptr;
}
}