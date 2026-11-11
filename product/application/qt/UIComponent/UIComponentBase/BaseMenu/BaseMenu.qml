import QtQuick
import QtQuick.Controls.Basic

Menu {
    id: control

    property color backgroundColor: palette.window
    property color borderColor: palette.mid
    property real borderWidth: 1
    property int borderRadius: 8
    property int menuPadding: 4

    padding: menuPadding
    overlap: 4

    background: Rectangle {
        implicitWidth: 200
        radius: control.borderRadius
        color: control.backgroundColor
        border.width: control.borderWidth
        border.color: control.borderColor
        layer.enabled: true
    }
}
