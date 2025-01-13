#include <UIDataStruct/UIColorSet.h>

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

UIColorsEnum UIColorSet::getColorEnum()
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

UIColors UIColorSet::getColors(UIColors::UIColorsEnum colorEnum)
{
    auto iter = std::find_if(mColors.cbegin(), mColors.cend(), [colorEnum](const auto uiColor){
        return colorEnum == uiColor->getColorEnum();
    });
    if (iter != mColors.cend())
    {
        return (*iter)->getFont(size, weight, isItalic);
    }
    UIData_LOG_WARN("can't find this ui family");
    return QFont("Segoe UI", 14, QFont::Normal, false);
}
}