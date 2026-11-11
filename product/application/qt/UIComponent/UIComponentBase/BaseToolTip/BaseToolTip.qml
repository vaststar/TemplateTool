import QtQuick
import QtQuick.Controls

ToolTip {
    id: control

    property color textColor: palette.toolTipText
    property color backgroundColor: palette.toolTipBase
    property color borderColor: palette.mid
    property real borderWidth: 1
    property int borderRadius: 4

    // Capture the cursor position when shown, then keep the tooltip still.
    property real cursorX: 0
    property real cursorY: 0
    property real _fixedX: 0
    property real _fixedY: 0
    x: _fixedX
    y: _fixedY

    delay: 500
    timeout: 3000
    margins: 4

    onVisibleChanged: {
        if (visible) {
            _fixedX = cursorX + 6
            _fixedY = cursorY + 14
        }
    }

    contentItem: Text {
        text: control.text
        font: control.font
        color: control.textColor
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        color: control.backgroundColor
        border.color: control.borderColor
        border.width: control.borderWidth
        radius: control.borderRadius
    }
}
