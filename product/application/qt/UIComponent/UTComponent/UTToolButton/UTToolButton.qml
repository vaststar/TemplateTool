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
    radius: 4
    borderWidth: 0
    clip: false

    font: UTComponentUtil.getUIFont(control.fontEnum)
    textColor: UTComponentUtil.getItemUIColor(control, control.fontColorEnum)
    backgroundColor: UTComponentUtil.getItemUIColor(control, control.backgroundColorEnum)
    borderColor: UTComponentUtil.getItemUIColor(control, control.borderColorEnum)

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
