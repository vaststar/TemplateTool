import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic

GroupBox {
    id: control

    property color titleColor: palette.windowText
    property color backgroundColor: palette.window
    property color borderColor: palette.mid
    property real borderWidth: 1
    property int borderRadius: 4

    label: Text {
        x: control.leftPadding
        text: control.title
        font: control.font
        color: control.titleColor
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        y: control.topPadding - control.bottomPadding
        width: parent.width
        height: parent.height - control.topPadding + control.bottomPadding
        color: control.backgroundColor
        border.color: control.borderColor
        border.width: control.borderWidth
        radius: control.borderRadius
    }
}
