import QtQuick
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTTextField - Themed single-line text input using design tokens.
 *
 * Supports normal, focused, readOnly, and placeholder states.
 */
BaseTextField {
    id: control

    // === Configurable font ===
    property var fontEnum: UIFontToken.Body_Text

    // === Theme colors ===
    readonly property color bgColor:           UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Background,  UIColorState.Normal)
    readonly property color textColor:         UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Text,        UIColorState.Normal)
    readonly property color borderNormal:      UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border,      UIColorState.Normal)
    readonly property color borderFocused:     UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border,      UIColorState.Focused)
    readonly property color placeholderColor:  UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Placeholder, UIColorState.Normal)

    color: control.textColor
    placeholderTextColor: control.placeholderColor
    font: UTComponentUtil.getUIFont(control.fontEnum)

    background: Rectangle {
        radius: control.borderRadius
        color: control.bgColor
        border.width: 1
        border.color: control.activeFocus ? control.borderFocused : control.borderNormal
    }

    UTFocusItem {
        target: control
        focusRadius: control.borderRadius
    }
}
