import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UTComponent

/**
 * Screen Capture Panel - Main panel with tabs for Screenshot, Recording, and Settings
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
            text: qsTr("🎬 Recording")
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

        // Screenshot Tab
        ScreenshotTab {
            controller: root.controller
        }

        // Recording Tab
        RecordingTab {
            controller: root.controller
        }

        // Settings Tab
        ScreenCaptureSettingsTab {
            controller: root.controller
        }
    }
}
