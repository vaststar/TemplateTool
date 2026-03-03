import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: root
    property MainWindowSideBarController controller: MainWindowSideBarController{}

    signal pageSelected(int pageId)

    // Background
    Rectangle {
        anchors.fill: parent
        color: "#FAFAFA"
    }
    
    // Right border
    Rectangle {
        anchors.right: parent.right
        width: 1
        height: parent.height
        color: "#E0E0E0"
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 6

        // Top navigation items
        ListView {
            id: topNavList
            Layout.fillWidth: true
            Layout.preferredHeight: contentHeight
            model: controller.topNavItems
            spacing: 4
            clip: true
            interactive: false

            delegate: NavItemDelegate {
                width: ListView.view.width
                isSelected: model.pageId === controller.currentPageId
                onClicked: {
                    controller.navigateTo(model.pageId)
                    root.pageSelected(model.pageId)
                }
            }
        }

        // Spacer
        Item { Layout.fillHeight: true }

        // Bottom navigation items
        ListView {
            id: bottomNavList
            Layout.fillWidth: true
            Layout.preferredHeight: contentHeight
            model: controller.bottomNavItems
            spacing: 4
            clip: true
            interactive: false

            delegate: NavItemDelegate {
                width: ListView.view.width
                isSelected: model.pageId === controller.currentPageId
                onClicked: {
                    controller.navigateTo(model.pageId)
                    root.pageSelected(model.pageId)
                }
            }
        }
    }

    // Nav item delegate component
    component NavItemDelegate: Rectangle {
        id: navItem
        height: 52
        radius: 8
        color: isSelected ? "#E3F2FD" : (mouseArea.containsMouse ? "#F5F5F5" : "transparent")

        required property int index
        required property int pageId
        required property string itemId
        required property string title
        required property var icon
        required property var iconSelected
        required property int badge
        required property int state

        property bool isSelected: false
        signal clicked()

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            spacing: 14

            // Icon
            Image {
                id: navIcon
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                source: UTComponentUtil.getImageResourcePath(
                    navItem.isSelected ? navItem.iconSelected : navItem.icon
                )
                sourceSize: Qt.size(24, 24)
                fillMode: Image.PreserveAspectFit
            }

            // Label
            Text {
                Layout.fillWidth: true
                text: navItem.title
                font.pixelSize: 15
                font.weight: navItem.isSelected ? Font.Medium : Font.Normal
                color: navItem.isSelected ? "#1976D2" : "#424242"
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }

            // Badge
            Rectangle {
                visible: navItem.badge > 0
                Layout.preferredWidth: Math.max(20, badgeText.implicitWidth + 8)
                Layout.preferredHeight: 20
                radius: 10
                color: "#F44336"

                Text {
                    id: badgeText
                    anchors.centerIn: parent
                    text: navItem.badge > 99 ? "99+" : navItem.badge.toString()
                    font.pixelSize: 12
                    font.weight: Font.Medium
                    color: "white"
                }
            }
        }

        // Disabled overlay
        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: "#80FFFFFF"
            visible: navItem.state === 1  // NavItemState::Disabled
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            enabled: navItem.state !== 1  // Not disabled
            onClicked: navItem.clicked()
        }
    }
}