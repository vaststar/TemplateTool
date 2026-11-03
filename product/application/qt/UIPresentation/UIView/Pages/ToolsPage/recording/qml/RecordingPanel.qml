import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import UTComponent
import UIView 1.0

/**
 * Recording Panel - Main panel with Recording controls and Settings tabs
 */
FocusScope {
    id: root

    property RecordingController controller: RecordingController {}

    Binding {
        target: root.controller.screenPermission
        property: "active"
        value: root.visible
               && root.Window.window !== null
               && root.Window.window.active
    }

    Binding {
        target: root.controller.microphonePermission
        property: "active"
        value: root.visible && root.controller.enableMicrophone
               && root.Window.window !== null
               && root.Window.window.active
    }

    Connections {
        target: root.controller.microphonePermission
        function onGrantedChanged() {
            if (root.controller.initialized) root.controller.refreshAudioDevices()
        }
    }

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
