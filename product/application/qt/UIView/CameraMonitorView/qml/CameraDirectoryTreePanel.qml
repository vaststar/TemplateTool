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

        // Right-click: resolve the hit node and open the context menu.
        onContextRequested: function(idx, pos) {
            const id   = idx.valid ? (treeView.model.data(idx, Qt.UserRole + 1) || "") : "";
            const type = idx.valid ? treeView.model.data(idx, Qt.UserRole + 3) : -1;
            contextMenu.openFor(id, type, pos);
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
        // Once a freshly-added node exists in the model, expand its parent and select it.
        function onNodeAdded(newId, parentId) {
            const tv = treeContainer.treeView;
            if (!tv) return;
            if (parentId !== "") {
                const pidx = tv.model.indexOfId(parentId);
                const pr = tv.rowAtIndex(pidx);
                if (pr >= 0) tv.expand(pr);
            }
            controller.selectNode(newId);
        }
    }

    // ---- Context menu ----
    // The controller owns the menu model (built from real add/remove permissions) and
    // decides which dialog an action opens. The panel only forwards the user's choice.
    UTDynamicMenu {
        id: contextMenu
        parent: treeContainer

        property string targetId: ""
        property int    targetType: -1   // -1 = blank/root, 0 = Group, 1 = Camera

        function openFor(id, type, pos) {
            targetId = id;
            targetType = type;
            model = root.controller.contextMenuModel(id, type);
            popup(pos.x, pos.y);
        }

        onItemTriggered: action => root.controller.handleContextAction(action, targetId, targetType)
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

        UTBusyIndicator {
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
