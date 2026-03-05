import QtQuick
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: contactDetailPanel

    required property ContactsPageController controller

    Text {
        anchors.centerIn: parent
        text: "选择联系人查看详情"
        font.pixelSize: 16
        color: UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Text, UIColorState.Normal)
    }
}
