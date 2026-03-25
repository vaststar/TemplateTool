import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UTComponent

/**
 * Recording Panel - Main panel with Recording controls and Settings tabs
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
        focus: true

        UTTabButton {
            text: qsTr("🎬 Gallery")
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
