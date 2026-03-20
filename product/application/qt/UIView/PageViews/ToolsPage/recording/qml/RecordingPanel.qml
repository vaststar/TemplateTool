import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UTComponent

/**
 * Recording Panel - Main panel with Recording controls and Settings tabs
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

        // Recording Tab
        RecordingContent {
            controller: root.controller
        }

        // Settings Tab
        RecordingSettingsTab {
            controller: root.controller
        }
    }
}
