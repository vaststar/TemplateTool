import QtQuick
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

BaseCalendarDatePicker {
    id: control

    property var backgroundColorEnum: UIColorToken.Datepicker_Background
    property var textColorEnum: UIColorToken.Datepicker_Text
    property var borderColorEnum: UIColorToken.Datepicker_Border
    property var todayColorEnum: UIColorToken.Datepicker_Today_Background
    property var selectedColorEnum: UIColorToken.Datepicker_Selected_Background

    font: UTComponentUtil.getUIFont(UIFontToken.Body_Text)
    navigationFont: UTComponentUtil.getUIFont(UIFontToken.Button_Text)

    backgroundColor: UTComponentUtil.getPlainUIColor(backgroundColorEnum, UIColorState.Normal)
    disabledBackgroundColor: UTComponentUtil.getPlainUIColor(backgroundColorEnum, UIColorState.Disabled)
    textColor: UTComponentUtil.getPlainUIColor(textColorEnum, UIColorState.Normal)
    disabledTextColor: UTComponentUtil.getPlainUIColor(textColorEnum, UIColorState.Disabled)
    borderColor: UTComponentUtil.getPlainUIColor(borderColorEnum, UIColorState.Normal)
    focusedBorderColor: UTComponentUtil.getPlainUIColor(borderColorEnum, UIColorState.Focused)
    todayBackgroundColor: UTComponentUtil.getPlainUIColor(todayColorEnum, UIColorState.Normal)
    hoveredDayBackgroundColor: UTComponentUtil.getPlainUIColor(todayColorEnum, UIColorState.Hovered)
    selectedBackgroundColor: UTComponentUtil.getPlainUIColor(selectedColorEnum, UIColorState.Selected)

    // Preserve the existing navigation button tokens independently of popup colors.
    navigationBackgroundColorFor: function(button) {
        return UTComponentUtil.getItemUIColor(button, UIColorToken.Datepicker_Background)
    }
    navigationTextColorFor: function(button) {
        return UTComponentUtil.getItemUIColor(button, control.textColorEnum)
    }
    navigationBorderColorFor: function(button) {
        return UTComponentUtil.getItemUIColor(button, UIColorToken.Button_Primary_Border)
    }

    focusIndicator: UTFocusItem {}

}
