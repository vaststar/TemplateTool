import QtQuick
import QtQuick.Controls.Basic

Switch {
    id: control

    property real trackWidth: 40
    property real trackHeight: 22
    property real thumbSize: 16
    property real thumbMargin: 3

    property color trackColor: palette.button
    property color thumbColor: palette.buttonText

    implicitWidth: control.trackWidth
    implicitHeight: control.trackHeight
    padding: 0

    indicator: Rectangle {
        width: control.trackWidth
        height: control.trackHeight
        radius: control.trackHeight / 2
        color: control.trackColor

        Behavior on color { ColorAnimation { duration: 150 } }

        Rectangle {
            id: thumb
            width: control.thumbSize
            height: control.thumbSize
            radius: control.thumbSize / 2
            y: (parent.height - height) / 2
            x: control.thumbMargin
               + control.visualPosition * (parent.width - width - 2 * control.thumbMargin)
            color: control.thumbColor

            Behavior on x { enabled: !control.down; NumberAnimation { duration: 150; easing.type: Easing.InOutQuad } }
            Behavior on color { ColorAnimation { duration: 150 } }
        }
    }

    contentItem: Item {}
}
