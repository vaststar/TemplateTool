import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTBusyIndicator - Themed indeterminate spinner with UT design tokens.
 *
 * Features:
 * - Track ring + orbiting dot, both colored from theme tokens
 * - Configurable size, ring thickness, dot size and spin cycle
 *
 * Usage:
 *   UTBusyIndicator { running: someLoadingFlag }
 *   UTBusyIndicator { indicatorSize: 48; cycleDuration: 1200 }
 */
BaseBusyIndicator {
    id: control

    property int indicatorSize: 36
    property int ringThickness: 3
    property int dotSize: 8
    property int cycleDuration: 900

    property var trackColorEnum: UIColorToken.Busy_Indicator_Track
    property var progressColorEnum: UIColorToken.Busy_Indicator_Progress

    implicitWidth: indicatorSize
    implicitHeight: indicatorSize

    contentItem: Item {
        implicitWidth: control.indicatorSize
        implicitHeight: control.indicatorSize

        Rectangle {
            anchors.fill: parent
            color: "transparent"
            border.width: control.ringThickness
            border.color: UTComponentUtil.getPlainUIColor(
                              control.trackColorEnum,
                              control.enabled ? UIColorState.Normal : UIColorState.Disabled)
            radius: width / 2
        }

        Item {
            anchors.fill: parent

            Rectangle {
                width: control.dotSize
                height: control.dotSize
                radius: width / 2
                anchors.horizontalCenter: parent.horizontalCenter
                y: -height / 2 + control.ringThickness / 2
                color: UTComponentUtil.getPlainUIColor(
                           control.progressColorEnum,
                           control.enabled ? UIColorState.Normal : UIColorState.Disabled)

                Behavior on color { ColorAnimation { duration: 100 } }
            }

            RotationAnimator on rotation {
                from: 0; to: 360
                duration: control.cycleDuration
                loops: Animation.Infinite
                running: control.running
            }
        }
    }
}
