import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

// Item-rooted, embeddable camera preview. One instance == one ViewModel.
// Host drives the lifecycle via controller.openLocalCamera / openNetworkCamera
// after calling initializeController(appContext).
Item {
    id: root
    property MediaCameraViewController controller: MediaCameraViewController {}

    implicitWidth: 758
    implicitHeight: 576

    Rectangle {
        anchors.fill: parent
        color: UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Background, UIColorState.Normal)
    }

    VideoOutput {
        id: videoOutput
        anchors.fill: parent
        fillMode: VideoOutput.PreserveAspectFit
        antialiasing: true
    }

    UTBusyIndicator {
        id: openingSpinner
        anchors.centerIn: parent
        z: 1
        visible: root.controller && root.controller.isOpening
        running: visible
    }

    UTText {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: openingSpinner.bottom
        anchors.topMargin: 8
        z: 1
        visible: openingSpinner.visible
        text: qsTr("Opening…")
        fontEnum: UIFontToken.Body_Text
    }

    Rectangle {
        id: errorOverlay
        anchors.fill: parent
        visible: root.controller && root.controller.openFailed
        // Semi-transparent overlay tied to the themed card background.
        color: Qt.rgba(
            UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Background, UIColorState.Normal).r,
            UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Background, UIColorState.Normal).g,
            UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Background, UIColorState.Normal).b,
            0.5)

        Column {
            anchors.centerIn: parent
            spacing: 6

            UTText {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "\u26A0"   // warning sign
                colorEnum: UIColorToken.Content_Error_Text
                fontEnum: UIFontToken.Heading_Text
            }
            UTText {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Failed to open camera")
                colorEnum: UIColorToken.Content_Error_Text
                fontEnum: UIFontToken.Body_Text
            }
        }
    }

    Connections {
        target: root.controller

        function onControllerInitialized() {
            root.controller.logInfo("MediaCameraView controller initialized")
        }

        function onShowCameraImage(img) {
            // Hook kept for future per-frame handling; intentionally a no-op.
        }
    }

    Component.onCompleted: {
        root.controller.logInfo("MediaCameraView qml load onCompleted")
        root.controller.videoSink = videoOutput.videoSink
    }
}
