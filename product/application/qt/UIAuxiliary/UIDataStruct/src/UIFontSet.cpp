#include <UIDataStruct/UIFont.h>

#include <string>
#include <map>
#include <QString>

#include "LoggerDefine.h"
namespace UIData{

    static const std::map<UIFont::UIFontFamily, std::string> UIFontFamilyMap = {
        {UIFont::UIFontFamily::SegoeUI, "Segoe UI"},
        {UIFont::UIFontFamily::Consolas, "Consolas"},
        {UIFont::UIFontFamily::SegoeUIEmoji, "Segoe UI Emoji"}
    };
UIFont::UIFont(QObject *parent)
    : QObject(parent)
    , mFontFamly(UIFontFamily::SegoeUI)
    , mFontFamilyName("Segoe UI")
{

}

UIFont::UIFont(UIFontFamily family, QObject *parent)
    : QObject(parent)
    , mFontFamly(family)
{
    if (auto iter = UIFontFamilyMap.find(family); iter != UIFontFamilyMap.end())
    {
        mFontFamilyName = iter->second;
    }
    else
    {
        mFontFamilyName = "Segoe UI";
    }
}

QFont UIFont::getFont(UIFontSize size, UIFontWeight weight, bool isItalic)
{
    int fontSize = 12;
    switch (size)
    {
    case UIFontSize::ExtraSmall:
        fontSize = 12;
        break;
    case UIFontSize::Small:
        fontSize = 14;
        break;
    case UIFontSize::MediumSmall:
        fontSize = 16;
        break;
    case UIFontSize::Medium:
        fontSize = 20;
        break;
    case UIFontSize::Large:
        fontSize = 26;
        break;
    case UIFontSize::ExtraLarge:
        fontSize = 40;
        break;
    default:
        break;
    }

    QFont::Weight fontWeight = QFont::Normal;
    switch (weight)
    {
    case UIFontWeight::Thin:
        fontWeight = QFont::Thin;
        break;
    case UIFontWeight::ExtraLight:
        fontWeight = QFont::ExtraLight;
        break;
    case UIFontWeight::Light:
        fontWeight = QFont::Light;
        break;
    case UIFontWeight::Normal:
        fontWeight = QFont::Normal;
        break;
    case UIFontWeight::Medium:
        fontWeight = QFont::Medium;
        break;
    case UIFontWeight::DemiBold:
        fontWeight = QFont::DemiBold;
        break;
    case UIFontWeight::Bold:
        fontWeight = QFont::Bold;
        break;
    case UIFontWeight::ExtraBold:
        fontWeight = QFont::ExtraBold;
        break;
    case UIFontWeight::Black:
        fontWeight = QFont::Black;
        break;
    default:
        break;
    }
    return QFont(QString::fromStdString(mFontFamilyName), fontSize, fontWeight, isItalic);
}

UIFontFamily UIFont::getFontFamily()
{
    return mFontFamly;
}

UIFontSet::UIFontSet(QObject *parent)
    : QObject(parent)
{

}

void UIFontSet::initFonts(const std::vector<UIFont*>& fonts)
{
    mFonts = fonts;
}

QFont UIFontSet::getFont(UIFont::UIFontFamily family, UIFont::UIFontSize size, UIFont::UIFontWeight weight, bool isItalic)
{
    auto iter = std::find_if(mFonts.cbegin(), mFonts.cend(), [family](const UIFont* uFont){
        return family == uFont->getFontFamily();
    });
    if (iter != mFonts.cend())
    {
        return (*iter)->getFont(size, weight);
    }
    UIData_LOG_WARN("can't find this ui family");
    return QFont("Segoe UI", 14, QFont::Normal, false);
}
}