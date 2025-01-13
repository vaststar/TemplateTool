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
        Button_Primary_Text,
        Button_Primary_Background,
        Button_Primary_Border
    };
    Q_ENUM(UIColorsEnum)
public:
    UIColors(UIColorsEnum colorEnum, const QColor& normal);
    UIColors(UIColorsEnum colorEnum, const QColor& normal, const QColor& hovered, const QColor& pressed, const QColor& disabled, const QColor& focused, const QColor& checked);

    UIColorsEnum getColorEnum();
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
    UIColors getColors(UIColors::UIColorsEnum);
private:
    std::vector<std::shared_ptr<UIColors>> mColors;
};
}