import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

BaseToolTip {
    id: control

    // Caller passes cursor position; tooltip snaps to cursor position at show time
    property real cursorX: 0
    property real cursorY: 0
    property real _fixedX: 0
    property real _fixedY: 0
    x: _fixedX
    y: _fixedY

    // Capture cursor position only when tooltip becomes visible
    onVisibleChanged: {
        if (visible) {
            _fixedX = cursorX + 6
            _fixedY = cursorY + 14
        }
    }

    contentItem: UTText {
        text: control.text
        fontEnum: UIFontToken.Caption_Text
        colorEnum: UIColorToken.Tooltip_Text
    }

    background: Rectangle {
        color: UTComponentUtil.getPlainUIColor(UIColorToken.Tooltip_Background, UIColorState.Normal)
        border.color: UTComponentUtil.getPlainUIColor(UIColorToken.Tooltip_Border, UIColorState.Normal)
        border.width: 1
        radius: 4
    }
}
