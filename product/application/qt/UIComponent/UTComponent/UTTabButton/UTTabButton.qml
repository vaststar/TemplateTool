import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTTabButton - Themed TabButton with UT design tokens and built-in focus ring.
 *
 * Usage:
 *   UTTabBar {
 *       UTTabButton { text: "Tab 1" }
 *       UTTabButton { text: "Tab 2" }
 *   }
 *
 * Customizable token properties:
 *   fontEnum, fontColorEnum, backgroundColorEnum, checkedBackgroundColorEnum, borderColorEnum
 */
BaseTabButton {
    id: control

    property var fontEnum: UIFontToken.Body_Text_Medium
    property var fontColorEnum: UIColorToken.Tab_Button_Text
    property var checkedFontColorEnum: UIColorToken.Tab_Button_Checked_Text
    property var backgroundColorEnum: UIColorToken.Tab_Button_Background
    property var checkedBackgroundColorEnum: UIColorToken.Tab_Button_Checked_Background
    property var borderColorEnum: UIColorToken.Button_Primary_Border

    font: UTComponentUtil.getUIFont(control.fontEnum)
    textColor: control.checked
               ? UTComponentUtil.getPlainUIColor(control.checkedFontColorEnum, UIColorState.Normal)
               : UTComponentUtil.getItemUIColor(control, control.fontColorEnum)
    borderColor: control.borderWidth > 0
                 ? UTComponentUtil.getItemUIColor(control, control.borderColorEnum)
                 : "transparent"
    backgroundColor: control.checked
                     ? UTComponentUtil.getPlainUIColor(control.checkedBackgroundColorEnum, UIColorState.Normal)
                     : UTComponentUtil.getItemUIColor(control, control.backgroundColorEnum)

    UTFocusItem {
        focusRadius: control.radius
        focusMargin: 1   // inset to avoid clipping by sibling tabs
    }
}
