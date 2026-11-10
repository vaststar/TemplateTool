import QtQuick
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

// Theme and focus decoration for the wheel date picker.
BaseWheelDatePicker {
    id: control

    property var backgroundColorEnum: UIColorToken.Datepicker_Background
    property var textColorEnum: UIColorToken.Datepicker_Text
    property var borderColorEnum: UIColorToken.Datepicker_Border
    property var selectedColorEnum: UIColorToken.Datepicker_Selected_Background

    font: UTComponentUtil.getUIFont(UIFontToken.Body_Text)
    backgroundColor: UTComponentUtil.getPlainUIColor(backgroundColorEnum, UIColorState.Normal)
    disabledBackgroundColor: UTComponentUtil.getPlainUIColor(backgroundColorEnum, UIColorState.Disabled)
    textColor: UTComponentUtil.getPlainUIColor(textColorEnum, UIColorState.Normal)
    disabledTextColor: UTComponentUtil.getPlainUIColor(textColorEnum, UIColorState.Disabled)
    borderColor: UTComponentUtil.getPlainUIColor(borderColorEnum, UIColorState.Normal)
    focusedBorderColor: UTComponentUtil.getPlainUIColor(borderColorEnum, UIColorState.Focused)
    selectedBandColor: UTComponentUtil.getPlainUIColor(selectedColorEnum, UIColorState.Selected)
    wheelCenteredPixelSize: 18
    wheelOffCenterPixelSize: 14

    UTFocusItem {
        target: control.popupIsOpen ? null : control
        focusRadius: control.borderRadius
    }

    UTFocusItem {
        parent: control.yearColumn
        focusRadius: control.borderRadius - 2
    }
    UTFocusItem {
        parent: control.monthColumn
        focusRadius: control.borderRadius - 2
    }
    UTFocusItem {
        parent: control.dayColumn
        focusRadius: control.borderRadius - 2
    }
}
