import QtQuick
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: placeholderPage
    property string pageTitle: "未实现"

    Text {
        anchors.centerIn: parent
        text: pageTitle
        font.pixelSize: 20
        color: UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Text, UIColorState.Normal)
    }
}
