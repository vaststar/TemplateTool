import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

BaseToolTip {
    id: control

    property var fontEnum: UIFontToken.Caption_Text
    property var fontColorEnum: UIColorToken.Tooltip_Text
    property var backgroundColorEnum: UIColorToken.Tooltip_Background
    property var borderColorEnum: UIColorToken.Tooltip_Border

    font: UTComponentUtil.getUIFont(control.fontEnum)
    textColor: UTComponentUtil.getPlainUIColor(control.fontColorEnum, UIColorState.Normal)
    backgroundColor: UTComponentUtil.getPlainUIColor(control.backgroundColorEnum, UIColorState.Normal)
    borderColor: UTComponentUtil.getPlainUIColor(control.borderColorEnum, UIColorState.Normal)
}
