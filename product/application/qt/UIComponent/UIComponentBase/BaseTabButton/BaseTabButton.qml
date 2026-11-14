import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic

TabButton {
    id: control

    property color textColor: palette.buttonText
    property color backgroundColor: palette.button
    property color borderColor: palette.mid
    property real radius: 4
    property real borderWidth: 0
    property int colorAnimationDuration: 150

    clip: false
    width: implicitWidth
    padding: 8
    leftPadding: 16
    rightPadding: 16

    contentItem: Text {
        text: control.text
        font: control.font
        color: control.textColor
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight

        Behavior on color {
            ColorAnimation { duration: control.colorAnimationDuration }
        }
    }

    background: Rectangle {
        radius: control.radius
        border.width: control.borderWidth
        border.color: control.borderColor
        color: control.backgroundColor

        Behavior on color {
            ColorAnimation { duration: control.colorAnimationDuration }
        }
    }
}
