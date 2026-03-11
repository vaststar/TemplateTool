import QtQuick
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: contactDetailPanel

    required property ContactsPageController controller

    property var contactInfo: controller.currentContactId 
        ? controller.getContactInfo(controller.currentContactId) 
        : null

    UTText {
        anchors.centerIn: parent
        text: contactInfo ? contactInfo.name : "选择联系人查看详情"
        fontEnum: UIFontToken.Window_Body_Primary
        colorEnum: UIColorToken.Sidebar_Item_Text
    }
}
