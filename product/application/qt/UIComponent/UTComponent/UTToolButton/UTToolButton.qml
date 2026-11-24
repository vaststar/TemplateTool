import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

BaseToolButton {
    id: control

    property string toolTipText: ""
    property int appearance: UTButton.Ghost
    property var fontColorEnum: appearance === UTButton.Primary ? UIColorToken.Button_Primary_Text
                               : appearance === UTButton.Destructive ? UIColorToken.Button_Destructive_Text
                               : UIColorToken.Button_Secondary_Text
    property var fontEnum: UIFontToken.Button_Text
    property var backgroundColorEnum: appearance === UTButton.Primary ? UIColorToken.Button_Primary_Background
                                     : appearance === UTButton.Destructive ? UIColorToken.Button_Destructive_Background
                                     : appearance === UTButton.Ghost ? UIColorToken.Button_Ghost_Background
                                     : UIColorToken.Button_Secondary_Background
    property var borderColorEnum: appearance === UTButton.Primary ? UIColorToken.Button_Primary_Border
                                 : appearance === UTButton.Destructive ? UIColorToken.Button_Destructive_Border
                                 : UIColorToken.Button_Secondary_Border
    radius: 4
    borderWidth: appearance === UTButton.Ghost ? 0 : 1
    clip: false

    Accessible.name: control.toolTipText || control.text

    font: UTComponentUtil.getUIFont(control.fontEnum)
    textColor: UTComponentUtil.getItemUIColor(control, control.fontColorEnum)
    backgroundColor: control.appearance === UTButton.Ghost
                     && (control.backgroundColorEnum === UIColorToken.Button_Ghost_Background
                         || control.backgroundColorEnum === UIColorToken.Button_Overlay_Background)
                     && (!control.enabled || (!control.hovered && !control.pressed && !control.checked))
                     ? "transparent"
                     : UTComponentUtil.getItemUIColor(control, control.backgroundColorEnum)
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
