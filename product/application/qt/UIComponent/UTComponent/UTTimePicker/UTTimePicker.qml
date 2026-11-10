import QtQuick
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

// Theme and focus decoration for the base time picker.
BaseTimePicker {
    id: control

    property var backgroundColorEnum: UIColorToken.Timepicker_Background
    property var textColorEnum: UIColorToken.Timepicker_Text
    property var borderColorEnum: UIColorToken.Timepicker_Border
    property var columnColorEnum: UIColorToken.Timepicker_Column_Background

    font: UTComponentUtil.getUIFont(UIFontToken.Body_Text)
    backgroundColor: UTComponentUtil.getPlainUIColor(backgroundColorEnum, UIColorState.Normal)
    disabledBackgroundColor: UTComponentUtil.getPlainUIColor(backgroundColorEnum, UIColorState.Disabled)
    textColor: UTComponentUtil.getPlainUIColor(textColorEnum, UIColorState.Normal)
    disabledTextColor: UTComponentUtil.getPlainUIColor(textColorEnum, UIColorState.Disabled)
    borderColor: UTComponentUtil.getPlainUIColor(borderColorEnum, UIColorState.Normal)
    focusedBorderColor: UTComponentUtil.getPlainUIColor(borderColorEnum, UIColorState.Focused)
    selectedBandColor: UTComponentUtil.getPlainUIColor(columnColorEnum, UIColorState.Selected)
    wheelCenteredPixelSize: 18
    wheelOffCenterPixelSize: 14

    UTFocusItem {
        target: control.popupIsOpen ? null : control
        focusRadius: control.borderRadius
    }

    UTFocusItem {
        parent: control.hourColumn
        focusRadius: control.borderRadius - 2
    }
    UTFocusItem {
        parent: control.minuteColumn
        focusRadius: control.borderRadius - 2
    }
    UTFocusItem {
        parent: control.secondColumn
        focusRadius: control.borderRadius - 2
    }
}
