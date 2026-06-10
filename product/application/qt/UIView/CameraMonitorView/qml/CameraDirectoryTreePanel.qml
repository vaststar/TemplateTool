import QtQuick
import QtQuick.Controls
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: root
    required property CameraMonitorViewController controller

    Rectangle {
        anchors.fill: parent
        color: UTComponentUtil.getPlainUIColor(UIColorToken.Main_Window_Background, UIColorState.Normal)
    }

    Rectangle {
        id: header
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        height: 40
        color: "transparent"

        UTText {
            anchors {
                left: parent.left
                leftMargin: 12
                verticalCenter: parent.verticalCenter
            }
            text: qsTr("Cameras")
            fontEnum: UIFontToken.Body_Text_Medium
            colorEnum: UIColorToken.Sidebar_Item_Text
            font.bold: true
        }
    }

    UTTreeView {
        id: treeContainer
        anchors {
            left: parent.left
            right: parent.right
            top: header.bottom
            bottom: parent.bottom
        }
        model: controller.cameraTreeModel
        visible: controller.loadState === CameraMonitorViewController.Ready
        expandAll: true

        onItemInvoked: function(idx) {
            var nodeId = treeView.model.data(idx, Qt.UserRole + 1);
            if (nodeId) controller.selectNode(nodeId);
        }

        acceptedMimeTypes: ["text/x-camera-node-id"]
        dropValidate: function(_mt, data, idx) {
            const targetId = idx.valid ? (treeView.model.data(idx, Qt.UserRole + 1) || "") : "";
            return controller.canDropOnNode(data, targetId);
        }
        onDropAccepted: function(_mt, data, idx) {
            const targetId = idx.valid ? (treeView.model.data(idx, Qt.UserRole + 1) || "") : "";
            controller.moveCameraNode(data, targetId);
        }

        delegate: UTTreeViewDelegate {
            text: (model.nodeType === 0 ? "📁 " : "🎥 ") + model.displayName
            highlighted: model.id === controller.currentCameraId
            dragMimeData: ({ "text/x-camera-node-id": model.id })
            onClicked: controller.selectNode(model.id)
        }
    }

    Connections {
        target: root.controller
        function onNodeMoved(newParentId) {
            if (newParentId === "") return;
            const tv = treeContainer.treeView;
            if (!tv) return;
            const idx = tv.model.indexOfId(newParentId);
            const r = tv.rowAtIndex(idx);
            if (r >= 0) tv.expand(r);
        }
    }

    // Status overlay while loading or after a load failure.
    Item {
        anchors {
            left: parent.left
            right: parent.right
            top: header.bottom
            bottom: parent.bottom
        }
        visible: controller.loadState !== CameraMonitorViewController.Ready

        BusyIndicator {
            anchors.centerIn: parent
            running: controller.loadState === CameraMonitorViewController.Loading
            visible: running
        }

        UTText {
            anchors.centerIn: parent
            visible: controller.loadState === CameraMonitorViewController.Error
            text: qsTr("Failed to load camera directory")
            fontEnum: UIFontToken.Body_Text
            colorEnum: UIColorToken.Sidebar_Item_Text
        }
    }
}
