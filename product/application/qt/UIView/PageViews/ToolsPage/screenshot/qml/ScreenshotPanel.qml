import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UTComponent

/**
 * Screenshot Panel - Main panel with Screenshot gallery and Settings tabs
 */
FocusScope {
    id: root

    required property var controller

    UTTabBar {
        id: tabBar
        anchors {
            top: parent.top
            topMargin: 4
            left: parent.left
            leftMargin: 4
            right: parent.right
        }
        focus: true   // forward initial focus into tab bar

        UTTabButton {
            text: qsTr("📷 Screenshot")
        }

        UTTabButton {
            text: qsTr("⚙️ Settings")
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
