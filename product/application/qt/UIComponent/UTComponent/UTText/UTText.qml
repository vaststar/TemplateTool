import QtQuick
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

BaseText {
    id: control

    required property var fontEnum
    required property var colorEnum
    property var colorState: UIColorState.Normal

    font: UTComponentUtil.getUIFont(fontEnum)
    color: UTComponentUtil.getPlainUIColor(colorEnum, colorState)
}
