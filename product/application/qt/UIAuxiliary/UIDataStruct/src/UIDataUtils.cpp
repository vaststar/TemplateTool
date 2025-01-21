#include <UIDataStruct/UIDataUtils.h>
#include <UIDataStruct/UIElementData.h>
#include <commonHead/ResourceLoader/ResourceLoaderModel.h>

commonHead::model::ColorItem UIDataUtils::convertUIColorEnumToVMColorItem(UIElementData::UIColorEnum  uiColorItem)
{
    switch (uiColorItem)
    {
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
