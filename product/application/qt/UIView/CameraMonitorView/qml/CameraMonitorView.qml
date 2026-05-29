import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

ApplicationWindow {
    id: root
    property CameraMonitorViewController controller: CameraMonitorViewController {}

    width: 1024
    height: 640
    title: qsTr("Camera Monitor")
    visible: true
    color: UTComponentUtil.getPlainUIColor(UIColorToken.Main_Window_Background, UIColorState.Normal)

    Component.onCompleted: {
        controller.logInfo("CameraMonitorView qml load onCompleted")
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Left: directory tree
        CameraDirectoryTreePanel {
            id: treePanel
            controller: root.controller
            Layout.preferredWidth: 280
            Layout.minimumWidth: 200
            Layout.fillHeight: true
        }

        Rectangle {
            width: 1
            Layout.fillHeight: true
            color: UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Background, UIColorState.Selected)
            opacity: 0.3
        }

        // Right: live preview
        CameraPreviewPanel {
            id: previewPanel
            controller: root.controller
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
