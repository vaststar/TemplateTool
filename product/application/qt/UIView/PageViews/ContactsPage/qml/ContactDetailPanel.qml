import QtQuick
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: contactDetailPanel

    required property ContactsPageController controller

    UTText {
        anchors.centerIn: parent
        text: "选择联系人查看详情"
        fontEnum: UIFontToken.Window_Body_Primary
        colorEnum: UIColorToken.Sidebar_Item_Text
    }
}
