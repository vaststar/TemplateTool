import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

BaseMenuItem {
    id: control

    property var fontEnum: UIFontToken.Body_Text
    property var shortcutFontEnum: UIFontToken.Caption_Text
    property var textColorEnum: UIColorToken.Menu_Text
    property var shortcutColorEnum: UIColorToken.Menu_Shortcut_Text
    property var backgroundColorEnum: UIColorToken.Menu_Item_Background

    font: UTComponentUtil.getUIFont(control.fontEnum)
    shortcutFont: UTComponentUtil.getUIFont(control.shortcutFontEnum)
    textColor: UTComponentUtil.getItemUIColor(control, control.textColorEnum)
    shortcutColor: UTComponentUtil.getPlainUIColor(control.shortcutColorEnum, UIColorState.Normal)
    backgroundColor: control.highlighted || control.hovered
                     ? UTComponentUtil.getPlainUIColor(control.backgroundColorEnum, UIColorState.Hovered)
                     : UTComponentUtil.getPlainUIColor(control.backgroundColorEnum, UIColorState.Normal)
    palette.windowText: control.textColor

    UTFocusItem {
        parent: control.background
        target: control.background
        delegateFocused: control.highlighted
        focusRadius: control.backgroundRadius
        focusMargin: -1
    }
}
