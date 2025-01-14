#include <UIDataStruct/UIColorSet.h>

#include "LoggerDefine.h"
namespace UIData{

UIColors::UIColors(UIColorsEnum colorEnum, const QColor& normal)
    : QObject(nullptr)
    , mColorEnum(colorEnum)
    , mNormal(normal)
{

}

UIColors::UIColors(UIColorsEnum colorEnum, const QColor& normal, const QColor& hovered, const QColor& pressed, const QColor& disabled, const QColor& focused, const QColor& checked)
    : QObject(nullptr)
    , mColorEnum(colorEnum)
    , mNormal(normal)
    , mHovered(hovered)
    , mPressed(pressed)
    , mDisabled(disabled)
    , mFocused(focused)
    , mChecked(checked)
{

}

QColor UIColors::getColor(UIColorState state) const
{ 
    switch (state)
    {
    case UIColorState::UIColorState_Normal:
        return mNormal;
    case UIColorState::UIColorState_Hovered:
        return mHovered;
    case UIColorState::UIColorState_Pressed:
        return mPressed;
    case UIColorState::UIColorState_Disabled:
        return mDisabled;
    case UIColorState::UIColorState_Focused:
        return mFocused;
    case UIColorState::UIColorState_Checked:
        return mChecked;
    default:
        return mNormal;
    }
}

UIColors::UIColorsEnum UIColors::getColorEnum() const
{
    return mColorEnum;
}

UIColorSet::UIColorSet(QObject* parent)
    : QObject(parent)
{

}

void UIColorSet::initColors(const std::vector<std::shared_ptr<UIColors>>& colors)
{
    mColors = colors;
}

std::shared_ptr<UIColors> UIColorSet::getUIColors(UIColors::UIColorsEnum colorEnum) const
{
    auto iter = std::find_if(mColors.cbegin(), mColors.cend(), [colorEnum](const auto uiColor){
        return colorEnum == uiColor->getColorEnum();
    });
    if (iter != mColors.cend())
    {
        return (*iter);
    }
    UIData_LOG_WARN("can't find this ui enum: " << static_cast<int>(colorEnum));
    return nullptr;
}
}