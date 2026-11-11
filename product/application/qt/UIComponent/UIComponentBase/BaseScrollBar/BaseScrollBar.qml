import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic

ScrollBar {
    id: control

    property real scrollBarWidth: 14
    property real thumbWidth: 6
    property real arrowSize: 18
    property bool showArrows: true

    property color trackColor: palette.base
    property color thumbColor: palette.mid
    property color decreaseArrowColor: palette.buttonText
    property color increaseArrowColor: palette.buttonText
    readonly property bool decreaseHovered: decreaseMA ? decreaseMA.containsMouse : false
    readonly property bool increaseHovered: increaseMA ? increaseMA.containsMouse : false

    policy: ScrollBar.AsNeeded
    implicitWidth: control.horizontal ? 200 : control.scrollBarWidth
    implicitHeight: control.horizontal ? control.scrollBarWidth : 200
    visible: control.policy !== ScrollBar.AlwaysOff
             && (control.policy === ScrollBar.AlwaysOn || control.size < 1.0)

    // No padding — arrows are drawn inside the background, not via ScrollBar padding.
    // ScrollBar.padding shifts the contentItem (thumb) area, which causes misalignment.
    padding: 0
    topPadding: !control.horizontal && control.showArrows ? control.arrowSize : 0
    bottomPadding: !control.horizontal && control.showArrows ? control.arrowSize : 0
    leftPadding: control.horizontal && control.showArrows ? control.arrowSize : 0
    rightPadding: control.horizontal && control.showArrows ? control.arrowSize : 0

    contentItem: Item {
        implicitWidth: control.thumbWidth
        implicitHeight: control.thumbWidth

        // ScrollBar owns the thumb's length and position; only inset its paint.
        Rectangle {
            anchors.centerIn: parent
            width: control.horizontal ? parent.width : control.thumbWidth
            height: control.horizontal ? control.thumbWidth : parent.height
            radius: control.thumbWidth / 2
            color: control.thumbColor

            Behavior on color {
                ColorAnimation { duration: 120 }
            }
        }
    }

    background: Rectangle {
        implicitWidth: control.horizontal ? 200 : control.scrollBarWidth
        implicitHeight: control.horizontal ? control.scrollBarWidth : 200
        color: control.trackColor
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
                // glyph size, intentionally not tokenized
                font.pixelSize: 12
                font.bold: true
                color: control.decreaseArrowColor
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
                // glyph size, intentionally not tokenized
                font.pixelSize: 12
                font.bold: true
                color: control.increaseArrowColor
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
