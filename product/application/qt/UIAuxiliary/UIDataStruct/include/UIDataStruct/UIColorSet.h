#pragma once 

#include <QObject>
#include <QColor>
#include <QString>
#include <QtQml>
#include <UIDataStruct/UIDataStructExport.h>

namespace UIData{


struct UIDataStruct_EXPORT UIColors: public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("only created in c++ code")
public:
    enum class UIColorsEnum{
        UIColorsEnum_Button_Primary_Text,
        UIColorsEnum_Button_Primary_Background,
        UIColorsEnum_Button_Primary_Border
    };

    enum class UIColorState{
        UIColorState_Normal,
        UIColorState_Hovered,
        UIColorState_Pressed,
        UIColorState_Disabled,
        UIColorState_Focused,
        UIColorState_Checked
    };
    Q_ENUM(UIColorsEnum)
    Q_ENUM(UIColorState)
public:
    UIColors(UIColorsEnum colorEnum, const QColor& normal);
    UIColors(UIColorsEnum colorEnum, const QColor& normal, const QColor& hovered, const QColor& pressed, const QColor& disabled, const QColor& focused, const QColor& checked);

    UIColorsEnum getColorEnum() const;

    QColor getColor(UIColorState state) const;
private:
    UIColorsEnum mColorEnum;
    QColor mNormal;
    QColor mHovered;
    QColor mPressed;
    QColor mDisabled;
    QColor mFocused;
    QColor mChecked;
};


struct UIDataStruct_EXPORT UIColorSet : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("only created in c++ code")
public:
    UIColorSet(QObject* parent = nullptr);
    void initColors(const std::vector<std::shared_ptr<UIColors>>& colors);
    std::shared_ptr<UIColors> getUIColors(UIColors::UIColorsEnum) const;
private:
    std::vector<std::shared_ptr<UIColors>> mColors;
};
}