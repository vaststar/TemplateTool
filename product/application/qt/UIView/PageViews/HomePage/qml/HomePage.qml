import QtQuick
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: homePage
    property HomePageController controller: HomePageController {}

    onVisibleChanged: {
        if (visible) {
            linkLabel.forceActiveFocus()
        }
    }

    Component.onCompleted: {
        if (visible) {
            Qt.callLater(function() { linkLabel.forceActiveFocus() })
        }
    }

    Column {
        anchors.centerIn: parent
        spacing: 16

        UTText {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "首页"
            fontEnum: UIFontToken.Heading_Text
            colorEnum: UIColorToken.Sidebar_Item_Text
        }

        UTLabel {
            id: linkLabel
            anchors.horizontalCenter: parent.horizontalCenter
            text: '点击访问 <a href="https://www.qt.io">Qt 官网</a> 了解更多'
            fontEnum: UIFontToken.Body_Text
            colorEnum: UIColorToken.Sidebar_Item_Text
            linkColorEnum: UIColorToken.Link_Text
            activeFocusOnTab: true
            onLinkActivated: (link) => Qt.openUrlExternally(link)
        }
    }
}
