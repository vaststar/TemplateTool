import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: root
    property MainWindowSideBarController controller: MainWindowSideBarController{}

    // Sidebar expand/collapse state
    property bool expanded: false
    readonly property int collapsedWidth: 50
    readonly property int expandedWidth: 120
    readonly property int animationDuration: 200

    implicitWidth: expanded ? expandedWidth : collapsedWidth

    Behavior on implicitWidth {
        NumberAnimation { duration: root.animationDuration; easing.type: Easing.InOutQuad }
    }

    // Background
    Rectangle {
        anchors.fill: parent
        color: UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Background, UIColorState.Normal)
    }

    // Right edge zone: border line + hover detection + expand/collapse arrow
    Rectangle {
        id: edgeZone
        anchors.right: parent.right
        width: 16
        height: parent.height
        color: "transparent"
        z: 5

        readonly property bool edgeHovered: edgeHoverHandler.hovered || arrowMouseArea.containsMouse || arrowHoverTimer.running

        // Visible 1px border line (synced with arrow visibility)
        Rectangle {
            anchors.right: parent.right
            width: 1
            height: parent.height
            color: UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Border, UIColorState.Normal)
            opacity: edgeZone.edgeHovered ? 1.0 : 0.0
            visible: opacity > 0
            Behavior on opacity { NumberAnimation { duration: 200; easing.type: Easing.InOutQuad } }
        }

        HoverHandler {
            id: edgeHoverHandler
            onHoveredChanged: if (hovered) arrowHoverTimer.restart()
        }

        Timer { id: arrowHoverTimer; interval: 800 }

        // Circular arrow button (centered on the right edge)
        Rectangle {
            id: edgeHandle
            width: 24; height: 24; radius: 12
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.right
            color: "transparent"
            border { color: UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Arrow_Border, UIColorState.Normal); width: 1 }
            opacity: edgeZone.edgeHovered ? 1.0 : 0.0
            visible: opacity > 0
            Behavior on opacity { NumberAnimation { duration: 200; easing.type: Easing.InOutQuad } }
            z: 10

            Text {
                anchors.centerIn: parent
                text: root.expanded ? "\u276E" : "\u276F"
                font.pixelSize: 12
                color: UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Arrow_Icon, UIColorState.Normal)
            }

            MouseArea {
                id: arrowMouseArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: root.expanded = !root.expanded
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: 8
        anchors.bottomMargin: 8
        anchors.leftMargin: 4
        anchors.rightMargin: 4
        spacing: 2

        // Top navigation items
        ListView {
            id: topNavList
            Layout.fillWidth: true
            Layout.preferredHeight: contentHeight
            model: controller.topNavItems
            spacing: 4
            clip: false
            interactive: false

            delegate: NavItemDelegate {
                width: ListView.view.width
                isSelected: pageId === controller.currentPageId
                showText: root.expanded
                animationDuration: root.animationDuration
                onClicked: controller.navigateTo(pageId)
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
            clip: false
            interactive: false

            delegate: NavItemDelegate {
                width: ListView.view.width
                isSelected: pageId === controller.currentPageId
                showText: root.expanded
                animationDuration: root.animationDuration
                onClicked: controller.navigateTo(pageId)
            }
        }
    }

}