import QtQuick
import QtQuick.Layouts
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: root
    property SettingsPageController controller
    property int currentNavId: 0

    width: 180

    Rectangle {
        anchors.fill: parent
        color: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Background, UIColorState.Normal)
    }

    // Right border
    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 1
        color: UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Border, UIColorState.Normal)
    }

    ListView {
        id: navList
        anchors.fill: parent
        anchors.margins: 8
        model: controller ? controller.navModel : null
        spacing: 2
        clip: true

        section.property: "section"
        section.delegate: Item {
            required property string section
            width: navList.width
            height: 28

            UTText {
                anchors.left: parent.left
                anchors.leftMargin: 8
                anchors.verticalCenter: parent.verticalCenter
                text: section
                fontEnum: UIFontToken.Section_Label
                colorEnum: UIColorToken.Sidebar_Item_Text
                font.capitalization: Font.AllUppercase
                color: Qt.alpha(UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Text, UIColorState.Normal), 0.6)
            }
        }

        delegate: Rectangle {
            id: navDelegate
            required property int index
            required property int navId
            required property string title

            width: navList.width
            height: 32
            radius: 6
            color: root.currentNavId === navDelegate.navId
                ? UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Background, UIColorState.Selected)
                : navDelegateArea.containsMouse
                    ? UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Background, UIColorState.Hovered)
                    : "transparent"

            UTText {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 16
                text: navDelegate.title
                fontEnum: UIFontToken.Body_Text
                colorEnum: UIColorToken.Sidebar_Item_Text
                colorState: root.currentNavId === navDelegate.navId
                    ? UIColorState.Selected
                    : UIColorState.Normal
            }

            MouseArea {
                id: navDelegateArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    root.currentNavId = navDelegate.navId
                }
            }
        }
    }
}
