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
    readonly property int _colorState: control.enabled ? UIColorState.Normal : UIColorState.Disabled
    readonly property color bgColor:           UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Background,  control._colorState)
    readonly property color textColor:         UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Text,        control._colorState)
    readonly property color borderNormal:      UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border,      control._colorState)
    readonly property color borderFocused:     UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border,      UIColorState.Focused)
    readonly property color placeholderColor:  UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Placeholder, control._colorState)

    color: control.textColor
    placeholderTextColor: control.placeholderColor
    font: UTComponentUtil.getUIFont(control.fontEnum)

    selectionColor: control.borderFocused
    selectedTextColor: control.textColor

    backgroundColor: control.bgColor
    borderColor: control.enabled && control.activeFocus ? control.borderFocused : control.borderNormal

    UTFocusItem {
        target: control
        focusRadius: control.borderRadius
    }
}
