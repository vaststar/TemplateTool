import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

BaseToolButton {
    id: control

    property string toolTipText: ""
    property var fontColorEnum: UIColorToken.Button_Primary_Text
    property var fontEnum: UIFontToken.Button_Text
    property var backgroundColorEnum: UIColorToken.Button_Primary_Background
    property var borderColorEnum: UIColorToken.Button_Primary_Border
    property real radius: 4
    property real borderWidth: 0
    clip: false

    contentItem: Text {
        text: control.text
        font: UTComponentUtil.getUIFont(control.fontEnum)
        color: UTComponentUtil.getItemUIColor(control, control.fontColorEnum)
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        anchors.fill: parent
        radius: control.radius
        border.width: control.borderWidth
        color: UTComponentUtil.getItemUIColor(control, control.backgroundColorEnum)
        border.color: UTComponentUtil.getItemUIColor(control, control.borderColorEnum)

        Behavior on color {
            ColorAnimation { duration: 150 }
        }
    }

    UTToolTip {
        text: control.toolTipText
        visible: control.toolTipText.length > 0 && control.hovered
        cursorX: _hoverHandler.point.position.x
        cursorY: _hoverHandler.point.position.y
    }

    HoverHandler {
        id: _hoverHandler
    }

    UTFocusItem {
        focusRadius: control.radius
    }
}
