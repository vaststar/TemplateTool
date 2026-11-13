import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTTextArea - Themed multi-line text input with UT design tokens.
 *
 * Usage:
 *   UTTextArea {
 *       text: myModel.content
 *       placeholderText: qsTr("Enter text here...")
 *       onTextChanged: myModel.content = text
 *   }
 */
BaseTextArea {
    id: control

    property var fontEnum: UIFontToken.Monospace_Text
    property var textColorEnum: UIColorToken.Content_Input_Text
    property var placeholderColorEnum: UIColorToken.Content_Input_Placeholder
    property var backgroundColorEnum: UIColorToken.Content_Input_Background
    property var borderColorEnum: UIColorToken.Content_Input_Border

    readonly property int _colorState: control.enabled ? UIColorState.Normal : UIColorState.Disabled

    font: UTComponentUtil.getUIFont(control.fontEnum)
    color: UTComponentUtil.getPlainUIColor(control.textColorEnum, control._colorState)
    placeholderTextColor: UTComponentUtil.getPlainUIColor(control.placeholderColorEnum, control._colorState)

    backgroundColor: UTComponentUtil.getPlainUIColor(control.backgroundColorEnum, control._colorState)
    borderColor: UTComponentUtil.getPlainUIColor(
                     control.borderColorEnum,
                     !control.enabled ? UIColorState.Disabled
                     : control.activeFocus ? UIColorState.Focused : UIColorState.Normal)

    UTFocusItem {
        focusRadius: control.borderRadius
    }
}
