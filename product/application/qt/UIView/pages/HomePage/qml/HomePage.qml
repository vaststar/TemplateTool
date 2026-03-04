import QtQuick
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: homePage
    property HomePageController controller: HomePageController {}

    Text {
        anchors.centerIn: parent
        text: "首页"
        font.pixelSize: 24
        color: UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Text, UIColorState.Normal)
    }
}
