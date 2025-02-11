#include <UIDataStruct/UIDataUtils.h>

#include <QQmlEngine>

#include <commonHead/ResourceLoader/ResourceLoaderModel.h>
#include <commonHead/ResourceLoader/ResourceLocalizedStringModel.h>
#include <commonhead/viewModels/ClientInfoViewModel/ClientInfoModel.h>

#include <UIDataStruct/UIElementData.h>
#include <UIDataStruct/UIStrings.h>
#include <UIDataStruct/UILanguage.h>

void UIDataUtils::registerMetaObject()
{
    UIElementData::registerMetaObject();
    UIStrings::registerMetaObject();
    UILanguage::registerMetaObject();
}

commonHead::model::ColorItem UIDataUtils::convertUIColorEnumToVMColorItem(UIElementData::UIColorEnum  uiColorItem)
{
    switch (uiColorItem)
    {
    case UIElementData::UIColorEnum::UIColorEnum_Item_Focus_Border:
        return commonHead::model::ColorItem::ColorItem_Item_Focus_Border;
    case UIElementData::UIColorEnum::UIColorEnum_Button_Primary_Text:
        return commonHead::model::ColorItem::ColorItem_Button_Primary_Text;
    case UIElementData::UIColorEnum::UIColorEnum_Button_Primary_Background:
        return commonHead::model::ColorItem::ColorItem_Button_Primary_Background;
    case UIElementData::UIColorEnum::UIColorEnum_Button_Primary_Border:
        return commonHead::model::ColorItem::ColorItem_Button_Primary_Border;
    default:
        return commonHead::model::ColorItem::ColorItem_Button_Primary_Text;
    }
}

commonHead::model::ColorItemState UIDataUtils::convertUIColorStateToVMColorItemState(UIElementData::UIColorState uiColorState)
{
    switch (uiColorState)
    {
    case UIElementData::UIColorState::UIColorState_Normal:
        return commonHead::model::ColorItemState::ColorItemState_Normal;
    case UIElementData::UIColorState::UIColorState_Hovered:
        return commonHead::model::ColorItemState::ColorItemState_Hovered;
    case UIElementData::UIColorState::UIColorState_Pressed:
        return commonHead::model::ColorItemState::ColorItemState_Pressed;
    case UIElementData::UIColorState::UIColorState_Disabled:
        return commonHead::model::ColorItemState::ColorItemState_Disabled;
    case UIElementData::UIColorState::UIColorState_Focused:
        return commonHead::model::ColorItemState::ColorItemState_Focused;
    case UIElementData::UIColorState::UIColorState_Checked:
        return commonHead::model::ColorItemState::ColorItemState_Checked;
    default:
        return commonHead::model::ColorItemState::ColorItemState_Normal;
    }
}

commonHead::model::FontFamily UIDataUtils::convertUIFontFamilyToVMFontFamily(UIElementData::UIFontFamily uiFontFamily)
{
    switch (uiFontFamily)
    {
    case UIElementData::UIFontFamily::UIFontFamily_SegoeUI:
        return commonHead::model::FontFamily::FontFamily_SegoeUI;
    case UIElementData::UIFontFamily::UIFontFamily_Consolas:
        return commonHead::model::FontFamily::FontFamily_Consolas;
    case UIElementData::UIFontFamily::UIFontFamily_SegoeUIEmoji:
        return commonHead::model::FontFamily::FontFamily_SegoeUIEmoji;
    default:
        return commonHead::model::FontFamily::FontFamily_SegoeUI;
    }
}

commonHead::model::FontSize UIDataUtils::convertUIFontSizeToVMFontSize(UIElementData::UIFontSize uiFontSize)
{
    switch (uiFontSize)
    {
    case UIElementData::UIFontSize::UIFontSize_ExtraSmall:
        return commonHead::model::FontSize::FontSize_ExtraSmall;
    case UIElementData::UIFontSize::UIFontSize_Small:
        return commonHead::model::FontSize::FontSize_Small;
    case UIElementData::UIFontSize::UIFontSize_MediumSmall:
        return commonHead::model::FontSize::FontSize_MediumSmall;
    case UIElementData::UIFontSize::UIFontSize_Medium:
        return commonHead::model::FontSize::FontSize_Medium;
    case UIElementData::UIFontSize::UIFontSize_Large:
        return commonHead::model::FontSize::FontSize_Large;
    case UIElementData::UIFontSize::UIFontSize_ExtraLarge:
        return commonHead::model::FontSize::FontSize_ExtraLarge;
    default:
        return commonHead::model::FontSize::FontSize_Small;
    }
}

commonHead::model::FontWeight UIDataUtils::convertUIFontWeightToVMFontWeight(UIElementData::UIFontWeight uiFontWeight)
{
    switch (uiFontWeight)
    {
    case UIElementData::UIFontWeight::UIFontWeight_Thin:
        return commonHead::model::FontWeight::FontWeight_Thin;
    case UIElementData::UIFontWeight::UIFontWeight_ExtraLight:
        return commonHead::model::FontWeight::FontWeight_ExtraLight;
    case UIElementData::UIFontWeight::UIFontWeight_Light:
        return commonHead::model::FontWeight::FontWeight_Light;
    case UIElementData::UIFontWeight::UIFontWeight_Normal:
        return commonHead::model::FontWeight::FontWeight_Normal;
    case UIElementData::UIFontWeight::UIFontWeight_Medium:
        return commonHead::model::FontWeight::FontWeight_Medium;
    case UIElementData::UIFontWeight::UIFontWeight_DemiBold:
        return commonHead::model::FontWeight::FontWeight_DemiBold;
    case UIElementData::UIFontWeight::UIFontWeight_Bold:
        return commonHead::model::FontWeight::FontWeight_Bold;
    case UIElementData::UIFontWeight::UIFontWeight_ExtraBold:
        return commonHead::model::FontWeight::FontWeight_ExtraBold;
    case UIElementData::UIFontWeight::UIFontWeight_Black:
        return commonHead::model::FontWeight::FontWeight_Black;
    default:
        return commonHead::model::FontWeight::FontWeight_Normal;
    }
}

commonHead::model::LocalizedString UIDataUtils::convertUILocalizedStringToVMLocalizedString(UIStrings::LocalizedString uiLocalizedString)
{
    switch (uiLocalizedString)
    {
    case UIStrings::LocalizedString::LocalizedString_None:
        return commonHead::model::LocalizedString::None;
    case UIStrings::LocalizedString::LocalizedString_OkButtonLabel:
        return commonHead::model::LocalizedString::OkButtonLabel;
    case UIStrings::LocalizedString::LocalizedString_CancelButtonLabel:
        return commonHead::model::LocalizedString::CancelButtonLabel;
    default:
        return commonHead::model::LocalizedString::None;
    }
}

commonHead::model::LocalizedStringWithParam UIDataUtils::convertUILocalizedStringParamToVMLocalizedStringParam(UIStrings::LocalizedStringWithParam uiLocalizedStringWithParam)
{
    switch (uiLocalizedStringWithParam)
    {
    case UIStrings::LocalizedStringWithParam::LocalizedStringWithParam_None:
        return commonHead::model::LocalizedStringWithParam::None;
    case UIStrings::LocalizedStringWithParam::LocalizedStringWithParam_TestParm:
        return commonHead::model::LocalizedStringWithParam::TestParm;
    default:
        return commonHead::model::LocalizedStringWithParam::None;
    }
}

commonHead::viewModels::model::LanguageType UIDataUtils::convertUILanguageToViewModelLanguage(UILanguage::LanguageType language)
{
    switch (language)
    {
    case UILanguage::LanguageType::LanguageType_ENGLISH:
        return commonHead::viewModels::model::LanguageType::ENGLISH;
    case UILanguage::LanguageType::LanguageType_CHINESE_SIMPLIFIED:
        return commonHead::viewModels::model::LanguageType::CHINESE_SIMPLIFIED;
    case UILanguage::LanguageType::LanguageType_CHINESE_TRADITIONAL:
        return commonHead::viewModels::model::LanguageType::CHINESE_TRADITIONAL;
    case UILanguage::LanguageType::LanguageType_FRENCH:
        return commonHead::viewModels::model::LanguageType::FRENCH;
    case UILanguage::LanguageType::LanguageType_GERMAN:
        return commonHead::viewModels::model::LanguageType::GERMAN;
    case UILanguage::LanguageType::LanguageType_ITALIAN:
        return commonHead::viewModels::model::LanguageType::ITALIAN;
    case UILanguage::LanguageType::LanguageType_SPANISH:
        return commonHead::viewModels::model::LanguageType::SPANISH;
    case UILanguage::LanguageType::LanguageType_PORTUGUESE:
        return commonHead::viewModels::model::LanguageType::PORTUGUESE;
    case UILanguage::LanguageType::LanguageType_JAPANESE:
        return commonHead::viewModels::model::LanguageType::JAPANESE;
    case UILanguage::LanguageType::LanguageType_KOREAN:
        return commonHead::viewModels::model::LanguageType::KOREAN;
    case UILanguage::LanguageType::LanguageType_RUSSIAN:
        return commonHead::viewModels::model::LanguageType::RUSSIAN;
    default:
        return commonHead::viewModels::model::LanguageType::ENGLISH;
    }
}

UILanguage::LanguageType UIDataUtils::convertViewModelLanguageToUILanguage(commonHead::viewModels::model::LanguageType language)
{
    switch (language)
    {
    case commonHead::viewModels::model::LanguageType::ENGLISH:
        return UILanguage::LanguageType::LanguageType_ENGLISH;
    case commonHead::viewModels::model::LanguageType::CHINESE_SIMPLIFIED:
        return UILanguage::LanguageType::LanguageType_CHINESE_SIMPLIFIED;
    case commonHead::viewModels::model::LanguageType::CHINESE_TRADITIONAL:
        return UILanguage::LanguageType::LanguageType_CHINESE_TRADITIONAL;
    case commonHead::viewModels::model::LanguageType::FRENCH:
        return UILanguage::LanguageType::LanguageType_FRENCH;
    case commonHead::viewModels::model::LanguageType::GERMAN:
        return UILanguage::LanguageType::LanguageType_GERMAN;
    case commonHead::viewModels::model::LanguageType::ITALIAN:
        return UILanguage::LanguageType::LanguageType_ITALIAN;
    case commonHead::viewModels::model::LanguageType::SPANISH:
        return UILanguage::LanguageType::LanguageType_SPANISH;
    case commonHead::viewModels::model::LanguageType::PORTUGUESE:
        return UILanguage::LanguageType::LanguageType_PORTUGUESE;
    case commonHead::viewModels::model::LanguageType::JAPANESE:
        return UILanguage::LanguageType::LanguageType_JAPANESE;
    case commonHead::viewModels::model::LanguageType::KOREAN:
        return UILanguage::LanguageType::LanguageType_KOREAN;
    case commonHead::viewModels::model::LanguageType::RUSSIAN:
        return UILanguage::LanguageType::LanguageType_RUSSIAN;
    default:
        return UILanguage::LanguageType::LanguageType_ENGLISH;
    }
}
