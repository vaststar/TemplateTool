import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTCheckBox - Themed checkbox with UT design tokens.
 *
 * Features:
 * - Rounded-rect indicator with checkmark
 * - Theme-aware colors via UIColorToken (light/dark)
 * - Smooth color animations
 * - Optional text label
 * - Focus ring via UTFocusItem
 *
 * Usage:
 *   UTCheckBox {
 *       text: qsTr("Enable feature")
 *       checked: myModel.enabled
 *       onToggled: myModel.enabled = checked
 *   }
 *
 *   // Without text (toggle-only)
 *   UTCheckBox {
 *       checked: controller.flag
 *       onToggled: controller.flag = checked
 *   }
 */
BaseCheckBox {
    id: control

    // === Configurable properties ===
    property real boxSize: 18
    property real boxRadius: 3
    property real checkmarkSize: 13

    property var boxColorEnum: UIColorToken.Checkbox_Box
    property var checkColorEnum: UIColorToken.Checkbox_Check
    property var textColorEnum: UIColorToken.Checkbox_Text
    property var borderColorEnum: UIColorToken.Content_Input_Border

    padding: 0
    spacing: 6

    indicator: Rectangle {
        implicitWidth: control.boxSize
        implicitHeight: control.boxSize
        x: control.leftPadding
        y: parent.height / 2 - height / 2
        radius: control.boxRadius
        color: {
            if (!control.enabled)
                return UTComponentUtil.getPlainUIColor(control.boxColorEnum, UIColorState.Disabled)
            if (control.checked)
                return UTComponentUtil.getPlainUIColor(control.boxColorEnum, UIColorState.Checked)
            if (control.pressed)
                return UTComponentUtil.getPlainUIColor(control.boxColorEnum, UIColorState.Pressed)
            if (control.hovered)
                return UTComponentUtil.getPlainUIColor(control.boxColorEnum, UIColorState.Hovered)
            return UTComponentUtil.getPlainUIColor(control.boxColorEnum, UIColorState.Normal)
        }
        border.width: control.checked ? 0 : 1
        border.color: UTComponentUtil.getPlainUIColor(
                          control.borderColorEnum,
                          control.activeFocus ? UIColorState.Focused : UIColorState.Normal)

        Behavior on color { ColorAnimation { duration: 150 } }

        Text {
            anchors.centerIn: parent
            text: "\u2713"
            font.pixelSize: control.checkmarkSize
            font.bold: true
            color: UTComponentUtil.getPlainUIColor(control.checkColorEnum, UIColorState.Normal)
            visible: control.checked
        }
    }

    contentItem: UTText {
        text: control.text
        fontEnum: UIFontToken.Body_Text
        colorEnum: control.textColorEnum
        verticalAlignment: Text.AlignVCenter
        leftPadding: control.text.length > 0 ? control.indicator.width + control.spacing : control.indicator.width
        visible: control.text.length > 0
    }

    UTFocusItem {
        focusRadius: control.boxRadius
    }
}
