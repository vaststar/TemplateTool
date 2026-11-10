import QtQuick
import QtQuick.Controls.Basic

Slider {
    id: control

    property int trackHeight: 4
    property int handleSize: 16
    property int handleBorderWidth: 1

    property color trackColor: palette.mid
    property color progressColor: palette.highlight
    property color handleColor: palette.button
    property color handleBorderColor: palette.mid

    live: true
    implicitWidth: 200
    implicitHeight: handleSize + 8

    background: Rectangle {
        x: control.leftPadding
        y: control.topPadding + control.availableHeight / 2 - height / 2
        width: control.availableWidth
        height: control.trackHeight
        radius: height / 2
        color: control.trackColor

        // Filled portion
        Rectangle {
            width: control.visualPosition * parent.width
            height: parent.height
            radius: parent.radius
            color: control.progressColor
        }
    }

    handle: Rectangle {
        x: control.leftPadding + control.visualPosition * (control.availableWidth - width)
        y: control.topPadding + control.availableHeight / 2 - height / 2
        width: control.handleSize
        height: control.handleSize
        radius: width / 2
        color: control.handleColor

        border.width: control.handleBorderWidth
        border.color: control.handleBorderColor

        Behavior on color { ColorAnimation { duration: 100 } }
    }
}
