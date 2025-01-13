#pragma once 

#include <string>
#include <vector>
#include <QObject>
#include <QFont>
#include <QtQml>
#include <UIDataStruct/UIDataStructExport.h>

namespace UIData{

struct UIDataStruct_EXPORT UIFont: public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("only created in c++ code")
public:
    enum class UIFontFamily{
        SegoeUI,
        Consolas,
        SegoeUIEmoji
    };

    enum class UIFontSize{
        ExtraSmall,     //12
        Small,          //14
        MediumSmall,    //16
        Medium,         //20
        Large,          //26
        ExtraLarge      //40
    };

    enum class UIFontWeight{
        Thin,
        ExtraLight,
        Light,
        Normal,
        Medium,
        DemiBold,
        Bold,
        ExtraBold,
        Black
    };
    Q_ENUM(UIFontFamily)
    Q_ENUM(UIFontSize)
    Q_ENUM(UIFontWeight)
public:
    UIFont(QObject *parent = nullptr);
    UIFont(UIFontFamily family, QObject *parent = nullptr);
    Q_INVOKABLE QFont getFont(UIFontSize size, UIFontWeight weight, bool isItalic);
    UIFontFamily getFontFamily();
private:
    UIFontFamily mFontFamly;
    std::string mFontFamilyName;
};


struct UIDataStruct_EXPORT UIFontSet : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("only created in c++ code")
public:
    UIFontSet(QObject* parent = nullptr);
    void initFonts(const std::vector<std::shared_ptr<UIFont>>& fonts);
    std::shared_ptr<UIFont> getUIFont(UIFont::UIFontFamily family);
private:
    std::vector<std::shared_ptr<UIFont>> mFonts;
};
}