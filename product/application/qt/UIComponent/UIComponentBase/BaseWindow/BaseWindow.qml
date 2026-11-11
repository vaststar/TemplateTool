import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: root

    property color borderColor: palette.mid
    property real borderWidth: 1

    visible: true

    Rectangle {
        parent: Overlay.overlay
        anchors.fill: parent
        z: 9999
        color: "transparent"
        border.width: root.borderWidth
        border.color: root.borderColor
    }
}
