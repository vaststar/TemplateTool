import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

BaseButton {
    id: control
    property var fontColorEnum: UIColorToken.Button_Primary_Text;
    property var fontEnum: UIFontToken.Button_Text;
    property var backgroundColorEnum: UIColorToken.Button_Primary_Background;
    property var borderColorEnum: UIColorToken.Button_Primary_Border;
    radius: Math.min(control.height, control.width)/2
    borderWidth: 1
    clip: false

    font: UTComponentUtil.getUIFont(control.fontEnum)
    textColor: UTComponentUtil.getItemUIColor(control, control.fontColorEnum)
    backgroundColor: UTComponentUtil.getItemUIColor(control, control.backgroundColorEnum)
    borderColor: UTComponentUtil.getItemUIColor(control, control.borderColorEnum)

    UTFocusItem{
        focusRadius: control.radius
    }
}
