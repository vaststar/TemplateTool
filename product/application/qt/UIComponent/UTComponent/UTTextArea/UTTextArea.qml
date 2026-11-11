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

    font: UTComponentUtil.getUIFont(control.fontEnum)
    color: UTComponentUtil.getPlainUIColor(control.textColorEnum, UIColorState.Normal)
    placeholderTextColor: UTComponentUtil.getPlainUIColor(control.placeholderColorEnum, UIColorState.Normal)

    backgroundColor: UTComponentUtil.getPlainUIColor(control.backgroundColorEnum, UIColorState.Normal)
    borderColor: UTComponentUtil.getPlainUIColor(
                     control.borderColorEnum,
                     control.activeFocus ? UIColorState.Focused : UIColorState.Normal)

    UTFocusItem {
        focusRadius: control.borderRadius
    }
}
