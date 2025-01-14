#include <UIDataStruct/UIFontSet.h>

#include <string>
#include <map>
#include <QString>

#include "LoggerDefine.h"
namespace UIData{

    static const std::map<UIFont::UIFontFamily, std::string> UIFontFamilyMap = {
        {UIFont::UIFontFamily::UIFontFamily_SegoeUI, "Segoe UI"},
        {UIFont::UIFontFamily::UIFontFamily_Consolas, "Consolas"},
        {UIFont::UIFontFamily::UIFontFamily_SegoeUIEmoji, "Segoe UI Emoji"}
    };
UIFont::UIFont(QObject *parent)
    : QObject(parent)
    , mFontFamly(UIFontFamily::UIFontFamily_SegoeUI)
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
    case UIFontSize::UIFontSize_ExtraSmall:
        fontSize = 12;
        break;
    case UIFontSize::UIFontSize_Small:
        fontSize = 14;
        break;
    case UIFontSize::UIFontSize_MediumSmall:
        fontSize = 16;
        break;
    case UIFontSize::UIFontSize_Medium:
        fontSize = 20;
        break;
    case UIFontSize::UIFontSize_Large:
        fontSize = 26;
        break;
    case UIFontSize::UIFontSize_ExtraLarge:
        fontSize = 40;
        break;
    default:
        break;
    }

    QFont::Weight fontWeight = QFont::Normal;
    switch (weight)
    {
    case UIFontWeight::UIFontWeight_Thin:
        fontWeight = QFont::Thin;
        break;
    case UIFontWeight::UIFontWeight_ExtraLight:
        fontWeight = QFont::ExtraLight;
        break;
    case UIFontWeight::UIFontWeight_Light:
        fontWeight = QFont::Light;
        break;
    case UIFontWeight::UIFontWeight_Normal:
        fontWeight = QFont::Normal;
        break;
    case UIFontWeight::UIFontWeight_Medium:
        fontWeight = QFont::Medium;
        break;
    case UIFontWeight::UIFontWeight_DemiBold:
        fontWeight = QFont::DemiBold;
        break;
    case UIFontWeight::UIFontWeight_Bold:
        fontWeight = QFont::Bold;
        break;
    case UIFontWeight::UIFontWeight_ExtraBold:
        fontWeight = QFont::ExtraBold;
        break;
    case UIFontWeight::UIFontWeight_Black:
        fontWeight = QFont::Black;
        break;
    default:
        break;
    }
    return QFont(QString::fromStdString(mFontFamilyName), fontSize, fontWeight, isItalic);
}

UIFont::UIFontFamily UIFont::getFontFamily()
{
    return mFontFamly;
}

UIFontSet::UIFontSet(QObject *parent)
    : QObject(parent)
{

}

void UIFontSet::initFonts(const std::vector<std::shared_ptr<UIFont>>& fonts)
{
    mFonts = fonts;
}

std::shared_ptr<UIFont> UIFontSet::getUIFont(UIFont::UIFontFamily family)
{
    auto iter = std::find_if(mFonts.cbegin(), mFonts.cend(), [family](const auto uFont){
        return family == uFont->getFontFamily();
    });
    if (iter != mFonts.cend())
    {
        return *iter;
    }
    UIData_LOG_WARN("can't find this ui family: " << static_cast<int>(family));
    return nullptr;
}
}