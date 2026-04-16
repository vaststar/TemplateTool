import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTSlider - Themed horizontal slider with UT design tokens.
 *
 * Features:
 * - Themed track (groove), filled progress, and round handle
 * - Smooth hover/press color transitions
 * - Focus ring via UTFocusItem
 *
 * Usage:
 *   UTSlider {
 *       from: 0; to: 100; value: 50
 *       onValueChanged: console.log(value)
 *   }
 */
BaseSlider {
    id: control

    property int trackHeight: 4
    property int handleSize: 16

    property var trackColorEnum: UIColorToken.Slider_Track
    property var progressColorEnum: UIColorToken.Slider_Progress
    property var handleColorEnum: UIColorToken.Slider_Handle

    implicitWidth: 200
    implicitHeight: handleSize + 8

    background: Rectangle {
        x: control.leftPadding
        y: control.topPadding + control.availableHeight / 2 - height / 2
        width: control.availableWidth
        height: control.trackHeight
        radius: height / 2
        color: UTComponentUtil.getPlainUIColor(control.trackColorEnum, UIColorState.Normal)

        // Filled portion
        Rectangle {
            width: control.visualPosition * parent.width
            height: parent.height
            radius: parent.radius
            color: UTComponentUtil.getPlainUIColor(
                       control.progressColorEnum,
                       control.enabled ? UIColorState.Normal : UIColorState.Disabled)
        }
    }

    handle: Rectangle {
        x: control.leftPadding + control.visualPosition * (control.availableWidth - width)
        y: control.topPadding + control.availableHeight / 2 - height / 2
        width: control.handleSize
        height: control.handleSize
        radius: width / 2
        color: {
            if (!control.enabled)
                return UTComponentUtil.getPlainUIColor(control.handleColorEnum, UIColorState.Disabled)
            if (control.pressed)
                return UTComponentUtil.getPlainUIColor(control.handleColorEnum, UIColorState.Pressed)
            if (control.hovered)
                return UTComponentUtil.getPlainUIColor(control.handleColorEnum, UIColorState.Hovered)
            return UTComponentUtil.getPlainUIColor(control.handleColorEnum, UIColorState.Normal)
        }

        Behavior on color { ColorAnimation { duration: 100 } }
    }

    UTFocusItem {
        focusRadius: control.handleSize / 2
    }
}
