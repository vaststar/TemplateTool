import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic

Popup {
    id: control

    property color backgroundColor: palette.window
    property color borderColor: palette.mid
    property real borderWidth: 1
    property int borderRadius: 4

    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    background: Rectangle {
        color: control.backgroundColor
        border.color: control.borderColor
        border.width: control.borderWidth
        radius: control.borderRadius
    }
}
