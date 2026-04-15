import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTSwitch - Themed toggle switch with UT design tokens.
 *
 * Features:
 * - Rounded track with sliding circular thumb
 * - Theme-aware colors via UIColorToken (light/dark)
 * - Smooth position and color animations
 * - Focus ring via UTFocusItem
 *
 * Usage:
 *   UTSwitch {
 *       checked: myModel.enabled
 *       onToggled: myModel.enabled = checked
 *   }
 */
BaseSwitch {
    id: control

    // === Configurable properties ===
    property real trackWidth: 40
    property real trackHeight: 22
    property real thumbSize: 16
    property real thumbMargin: 3

    property var trackColorEnum: UIColorToken.Switch_Track
    property var thumbColorEnum: UIColorToken.Switch_Thumb

    implicitWidth: control.trackWidth
    implicitHeight: control.trackHeight
    padding: 0

    indicator: Rectangle {
        width: control.trackWidth
        height: control.trackHeight
        radius: control.trackHeight / 2
        color: {
            if (!control.enabled)
                return UTComponentUtil.getPlainUIColor(control.trackColorEnum, UIColorState.Disabled)
            if (control.checked)
                return UTComponentUtil.getPlainUIColor(control.trackColorEnum, UIColorState.Checked)
            if (control.pressed)
                return UTComponentUtil.getPlainUIColor(control.trackColorEnum, UIColorState.Pressed)
            if (control.hovered)
                return UTComponentUtil.getPlainUIColor(control.trackColorEnum, UIColorState.Hovered)
            return UTComponentUtil.getPlainUIColor(control.trackColorEnum, UIColorState.Normal)
        }

        Behavior on color { ColorAnimation { duration: 150 } }

        Rectangle {
            id: thumb
            width: control.thumbSize
            height: control.thumbSize
            radius: control.thumbSize / 2
            y: (parent.height - height) / 2
            x: control.checked
               ? parent.width - width - control.thumbMargin
               : control.thumbMargin
            color: UTComponentUtil.getPlainUIColor(
                       control.thumbColorEnum,
                       control.enabled ? UIColorState.Normal : UIColorState.Disabled)

            Behavior on x { NumberAnimation { duration: 150; easing.type: Easing.InOutQuad } }
            Behavior on color { ColorAnimation { duration: 150 } }
        }
    }

    contentItem: Item {}

    UTFocusItem {
        focusRadius: control.trackHeight / 2
    }
}
