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

    TabBar {
        id: tabBar
        anchors {
            top: parent.top
            topMargin: 4
            left: parent.left
            leftMargin: 4
            right: parent.right
        }
        focus: true   // forward initial focus into tab bar
        clip: false

        TabButton {
            id: screenshotTab
            text: qsTr("📷 Screenshot")
            width: implicitWidth
            z: activeFocus ? 10 : 0

            UTFocusItem {
                target: screenshotTab
                focusRadius: 4
            }
        }

        TabButton {
            id: settingsTab
            text: qsTr("⚙️ Settings")
            width: implicitWidth
            z: activeFocus ? 10 : 0

            UTFocusItem {
                target: settingsTab
                focusRadius: 4
            }
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
