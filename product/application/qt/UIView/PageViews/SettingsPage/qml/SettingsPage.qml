import QtQuick
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: settingsPage
    property SettingsPageController controller: SettingsPageController {}

    Text {
        anchors.centerIn: parent
        text: "设置"
        font.pixelSize: 24
        color: UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Text, UIColorState.Normal)
    }
}
