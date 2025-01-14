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
        UIFontFamily_SegoeUI,
        UIFontFamily_Consolas,
        UIFontFamily_SegoeUIEmoji
    };

    enum class UIFontSize{
        UIFontSize_ExtraSmall,     //12
        UIFontSize_Small,          //14
        UIFontSize_MediumSmall,    //16
        UIFontSize_Medium,         //20
        UIFontSize_Large,          //26
        UIFontSize_ExtraLarge      //40
    };

    enum class UIFontWeight{
        UIFontWeight_Thin,
        UIFontWeight_ExtraLight,
        UIFontWeight_Light,
        UIFontWeight_Normal,
        UIFontWeight_Medium,
        UIFontWeight_DemiBold,
        UIFontWeight_Bold,
        UIFontWeight_ExtraBold,
        UIFontWeight_Black
    };
    Q_ENUM(UIFontFamily)
    Q_ENUM(UIFontSize)
    Q_ENUM(UIFontWeight)
public:
    UIFont(QObject *parent = nullptr);
    UIFont(UIFontFamily family, QObject *parent = nullptr);
    QFont getFont(UIFontSize size, UIFontWeight weight, bool isItalic);
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