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

    property var fontEnum: UIFontToken.Button_Text
    property var fontColorEnum: UIColorToken.Button_Primary_Text
    property var backgroundColorEnum: UIColorToken.Button_Primary_Background
    property var checkedBackgroundColorEnum: UIColorToken.Button_Primary_Background
    property var borderColorEnum: UIColorToken.Button_Primary_Border
    property real radius: 4
    property real borderWidth: 0

    clip: false
    width: implicitWidth
    padding: 8
    leftPadding: 16
    rightPadding: 16

    contentItem: Text {
        text: control.text
        font: UTComponentUtil.getUIFont(control.fontEnum)
        color: UTComponentUtil.getItemUIColor(control, control.fontColorEnum)
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight

        Behavior on color {
            ColorAnimation { duration: 150 }
        }
    }

    background: Rectangle {
        radius: control.radius
        border.width: control.borderWidth
        border.color: control.borderWidth > 0
                      ? UTComponentUtil.getItemUIColor(control, control.borderColorEnum)
                      : "transparent"
        color: {
            if (control.checked)
                return UTComponentUtil.getPlainUIColor(control.checkedBackgroundColorEnum, UIColorState.Checked)
            return UTComponentUtil.getItemUIColor(control, control.backgroundColorEnum)
        }

        Behavior on color {
            ColorAnimation { duration: 150 }
        }
    }

    UTFocusItem {
        focusRadius: control.radius
        focusMargin: 1   // inset to avoid clipping by sibling tabs
    }
}
