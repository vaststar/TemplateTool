import QtQuick
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

BaseText {
    id: control

    property var fontEnum: UIFontToken.Body_Text
    property var colorEnum: UIColorToken.Content_Text
    property var colorState: UIColorState.Normal

    font: UTComponentUtil.getUIFont(fontEnum)
    color: UTComponentUtil.getPlainUIColor(colorEnum, colorState)
}
