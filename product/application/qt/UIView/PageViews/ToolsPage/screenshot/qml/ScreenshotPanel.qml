import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UTComponent

/**
 * Screenshot Panel - Main panel with Screenshot gallery and Settings tabs
 */
Item {
    id: root

    required property var controller

    TabBar {
        id: tabBar
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        TabButton {
            text: qsTr("📷 Screenshot")
            width: implicitWidth
        }

        TabButton {
            text: qsTr("⚙️ Settings")
            width: implicitWidth
        }
    }

    StackLayout {
        anchors {
            top: tabBar.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            margins: 16
        }
        currentIndex: tabBar.currentIndex

        // Screenshot Gallery Tab
        ScreenshotGallery {
            controller: root.controller
        }

        // Settings Tab
        ScreenshotSettingsTab {
            controller: root.controller
        }
    }
}
