import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTProgressBar - Themed progress bar with UT design tokens.
 *
 * Features:
 * - Themed track and progress fill using design tokens
 * - Smooth color transitions based on enabled state
 * - Customizable height and colors
 *
 * Usage:
 *   UTProgressBar {
 *       from: 0; to: 1.0; value: 0.5
 *   }
 */
BaseProgressBar {
    id: control

    property int barHeight: 8

    property var trackColorEnum: UIColorToken.Slider_Track
    property var progressColorEnum: UIColorToken.Slider_Progress

    implicitWidth: 200
    implicitHeight: barHeight

    from: 0
    to: 1.0

    background: Rectangle {
        implicitHeight: control.barHeight
        color: UTComponentUtil.getPlainUIColor(control.trackColorEnum, UIColorState.Normal)
        radius: height / 2
    }

    contentItem: Item {
        implicitWidth: 200
        implicitHeight: control.barHeight

        Rectangle {
            width: control.visualPosition * parent.width
            height: parent.height
            radius: height / 2
            color: UTComponentUtil.getPlainUIColor(
                       control.progressColorEnum,
                       control.enabled ? UIColorState.Normal : UIColorState.Disabled)

            Behavior on color { ColorAnimation { duration: 100 } }
        }
    }
}
