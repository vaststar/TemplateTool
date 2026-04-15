import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTScrollBar - Themed ScrollBar with arrow buttons and UT design tokens.
 *
 * Features:
 * - Narrow track with rounded thumb
 * - Top/bottom (or left/right) arrow buttons for step scrolling
 * - Hover/pressed visual feedback with smooth animations
 * - showArrows property to toggle arrow buttons
 *
 * Usage:
 *   ListView {
 *       ScrollBar.vertical: UTScrollBar {}
 *   }
 */
BaseScrollBar {
    id: control

    // === Configurable properties ===
    property real scrollBarWidth: 14
    property real thumbWidth: 6
    property real arrowSize: 18
    property bool showArrows: true

    property var trackColorEnum: UIColorToken.Scrollbar_Track
    property var thumbColorEnum: UIColorToken.Scrollbar_Thumb
    property var arrowColorEnum: UIColorToken.Scrollbar_Arrow

    implicitWidth: control.horizontal ? 200 : control.scrollBarWidth
    implicitHeight: control.horizontal ? control.scrollBarWidth : 200
    visible: control.size < 1.0

    // No padding — arrows are drawn inside the background, not via ScrollBar padding.
    // ScrollBar.padding shifts the contentItem (thumb) area, which causes misalignment.
    padding: 0
    topPadding: !control.horizontal && control.showArrows ? control.arrowSize : 0
    bottomPadding: !control.horizontal && control.showArrows ? control.arrowSize : 0
    leftPadding: control.horizontal && control.showArrows ? control.arrowSize : 0
    rightPadding: control.horizontal && control.showArrows ? control.arrowSize : 0

    contentItem: Rectangle {
        implicitWidth: control.horizontal ? 0 : control.thumbWidth
        implicitHeight: control.horizontal ? control.thumbWidth : 0

        // Center the narrow thumb within the scrollbar width
        x: control.horizontal ? 0 : (control.scrollBarWidth - control.thumbWidth) / 2
        width: control.horizontal ? parent.width : control.thumbWidth

        radius: control.thumbWidth / 2
        color: UTComponentUtil.getItemUIColor(control, control.thumbColorEnum)

        Behavior on color {
            ColorAnimation { duration: 120 }
        }
    }

    background: Rectangle {
        implicitWidth: control.horizontal ? 200 : control.scrollBarWidth
        implicitHeight: control.horizontal ? control.scrollBarWidth : 200
        color: UTComponentUtil.getPlainUIColor(control.trackColorEnum, UIColorState.Normal)
        radius: control.scrollBarWidth / 2

        // Up / Left arrow button
        Rectangle {
            id: decreaseBtn
            visible: control.showArrows
            width: control.horizontal ? control.arrowSize : parent.width
            height: control.horizontal ? parent.height : control.arrowSize
            anchors {
                top: control.horizontal ? parent.top : parent.top
                left: control.horizontal ? parent.left : parent.left
            }
            color: "transparent"
            radius: control.scrollBarWidth / 2

            Text {
                anchors.centerIn: parent
                text: control.horizontal ? "\u2039" : "\u2303"   // ‹ or ⌃
                font.pixelSize: 12
                font.bold: true
                color: UTComponentUtil.getPlainUIColor(control.arrowColorEnum,
                           decreaseMA.containsMouse ? UIColorState.Hovered : UIColorState.Normal)
                Behavior on color { ColorAnimation { duration: 100 } }
            }

            MouseArea {
                id: decreaseMA
                anchors.fill: parent
                hoverEnabled: true
                onClicked: control.decrease()
            }
        }

        // Down / Right arrow button
        Rectangle {
            id: increaseBtn
            visible: control.showArrows
            width: control.horizontal ? control.arrowSize : parent.width
            height: control.horizontal ? parent.height : control.arrowSize
            anchors {
                bottom: control.horizontal ? parent.bottom : parent.bottom
                right: control.horizontal ? parent.right : parent.right
            }
            color: "transparent"
            radius: control.scrollBarWidth / 2

            Text {
                anchors.centerIn: parent
                text: control.horizontal ? "\u203A" : "\u2304"   // › or ⌄
                font.pixelSize: 12
                font.bold: true
                color: UTComponentUtil.getPlainUIColor(control.arrowColorEnum,
                           increaseMA.containsMouse ? UIColorState.Hovered : UIColorState.Normal)
                Behavior on color { ColorAnimation { duration: 100 } }
            }

            MouseArea {
                id: increaseMA
                anchors.fill: parent
                hoverEnabled: true
                onClicked: control.increase()
            }
        }
    }
}
