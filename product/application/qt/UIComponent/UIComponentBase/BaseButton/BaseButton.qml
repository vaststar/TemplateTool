import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic

Button {
    id: control

    property color textColor: palette.buttonText
    property color backgroundColor: palette.button
    property color borderColor: palette.mid
    property real radius: 4
    property real borderWidth: 1

    flat: true

    contentItem: Text {
        text: control.text
        font: control.font
        color: control.textColor
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        anchors.fill: parent
        radius: control.radius
        border.width: control.borderWidth
        color: control.backgroundColor
        border.color: control.borderColor

        Behavior on color {
            ColorAnimation { duration: 150 }
        }
    }
}
