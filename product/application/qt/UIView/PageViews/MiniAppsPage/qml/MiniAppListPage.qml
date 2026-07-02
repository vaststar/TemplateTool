import QtQuick
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: miniAppsPage
    property MiniAppListPageController controller: MiniAppListPageController {}

    GridView {
        id: appGrid
        anchors.fill: parent
        anchors.margins: 24
        cellWidth: 140
        cellHeight: 140
        model: miniAppsPage.controller.miniApps

        delegate: Item {
            id: appCell
            width: appGrid.cellWidth
            height: appGrid.cellHeight
            required property var modelData

            Column {
                anchors.centerIn: parent
                spacing: 10

                Rectangle {
                    width: 72
                    height: 72
                    radius: 16
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Background, UIColorState.Normal)

                    UTText {
                        anchors.centerIn: parent
                        text: appCell.modelData.name.length > 0 ? appCell.modelData.name.charAt(0) : "?"
                        fontEnum: UIFontToken.Subtitle_Text
                        colorEnum: UIColorToken.Sidebar_Item_Text
                    }
                }

                UTText {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: appCell.modelData.name
                    fontEnum: UIFontToken.Subtitle_Text
                    colorEnum: UIColorToken.Sidebar_Item_Text
                }
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: miniAppsPage.controller.launchMiniApp(appCell.modelData.id)
            }
        }
    }
}
