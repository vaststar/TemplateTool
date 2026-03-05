import QtQuick
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

BaseIconImage {
    id: control

    required property var imageSourceEnum

    property var colorEnum: UIColorToken.Sidebar_Item_Text
    property var colorState: UIColorState.Normal

    source: UTComponentUtil.getImageResourcePath(imageSourceEnum)
    color: UTComponentUtil.getPlainUIColor(colorEnum, colorState)
}
