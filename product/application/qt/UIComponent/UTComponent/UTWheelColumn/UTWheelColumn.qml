import QtQuick
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

// Themed wheel column shared by the time and wheel date pickers.
BaseWheelColumn {
    id: wheel

    property var textColorEnum: UIColorToken.Timepicker_Text
    // Retained for existing callers; the focus outline uses UTFocusItem's token.
    property var borderColorEnum: UIColorToken.Timepicker_Border
    property real borderRadius: 6

    font: UTComponentUtil.getUIFont(UIFontToken.Body_Text)
    textColor: UTComponentUtil.getPlainUIColor(textColorEnum, UIColorState.Normal)
    disabledTextColor: UTComponentUtil.getPlainUIColor(textColorEnum, UIColorState.Disabled)
    centeredFont: {
        const style = Object.assign({}, wheel.font)
        delete style.weight
        style.bold = true
        return Qt.font(style)
    }
    offCenterFont: {
        const style = Object.assign({}, wheel.font)
        delete style.weight
        style.bold = false
        return Qt.font(style)
    }
    disabledOpacity: 0.35
    minimumOpacity: 0.4
    displacementOpacity: 0.28
    centeredPixelSize: 18
    offCenterPixelSize: 14

    UTFocusItem {
        target: wheel
        delegateFocused: wheel.activeFocus
        focusRadius: wheel.borderRadius - 2
    }
}
