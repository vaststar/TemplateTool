import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

BaseButton {
    id: control

    enum Appearance { Primary, Secondary, Ghost, Destructive }

    // Appearance is independent of keyboard focus and the action's result.
    property int appearance: UTButton.Primary
    property var fontColorEnum: appearance === UTButton.Primary ? UIColorToken.Button_Primary_Text
                               : appearance === UTButton.Destructive ? UIColorToken.Button_Destructive_Text
                               : UIColorToken.Button_Secondary_Text
    property var fontEnum: UIFontToken.Button_Text;
    property var backgroundColorEnum: appearance === UTButton.Primary ? UIColorToken.Button_Primary_Background
                                     : appearance === UTButton.Destructive ? UIColorToken.Button_Destructive_Background
                                     : appearance === UTButton.Ghost ? UIColorToken.Button_Ghost_Background
                                     : UIColorToken.Button_Secondary_Background
    property var borderColorEnum: appearance === UTButton.Primary ? UIColorToken.Button_Primary_Border
                                 : appearance === UTButton.Destructive ? UIColorToken.Button_Destructive_Border
                                 : UIColorToken.Button_Secondary_Border
    radius: Math.min(control.height, control.width)/2
    borderWidth: appearance === UTButton.Ghost ? 0 : 1
    clip: false

    font: UTComponentUtil.getUIFont(control.fontEnum)
    textColor: UTComponentUtil.getItemUIColor(control, control.fontColorEnum)
    // Ghost and fixed-dark overlay palettes only fill interaction states.
    // Other custom background tokens retain their normal fill.
    backgroundColor: control.appearance === UTButton.Ghost
                     && (control.backgroundColorEnum === UIColorToken.Button_Ghost_Background
                         || control.backgroundColorEnum === UIColorToken.Button_Overlay_Background)
                     && (!control.enabled || (!control.hovered && !control.pressed && !control.checked))
                     ? "transparent"
                     : UTComponentUtil.getItemUIColor(control, control.backgroundColorEnum)
    borderColor: UTComponentUtil.getItemUIColor(control, control.borderColorEnum)

    UTFocusItem{
        focusRadius: control.radius
    }
}
