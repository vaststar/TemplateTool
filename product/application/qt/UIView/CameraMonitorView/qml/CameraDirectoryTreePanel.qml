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

        delegate: TreeViewDelegate {
            id: rowItem
            implicitWidth: treeView.width
            indentation: 16
            leftMargin: 8
            spacing: 6
            topPadding: 4
            bottomPadding: 4
            focusPolicy: Qt.NoFocus
            z: (treeView.currentRow === row && treeView.activeFocus) ? 1 : 0

            readonly property string nodeId: model.id
            readonly property bool isSelected: nodeId === controller.currentCameraId

            // Off-screen chip used as the drag thumbnail.
            Item {
                id: dragPreview
                width: previewLabel.implicitWidth + 16
                height: previewLabel.implicitHeight + 8
                visible: false
                layer.enabled: true
                Rectangle {
                    anchors.fill: parent
                    radius: 4
                    color: UTComponentUtil.getPlainUIColor(
                        UIColorToken.Sidebar_Item_Background, UIColorState.Selected)
                    opacity: 0.9
                }
                UTLabel {
                    id: previewLabel
                    anchors.centerIn: parent
                    text: (model.nodeType === 0 ? "📁 " : "🎥 ") + model.displayName
                    fontEnum: UIFontToken.Body_Text
                    colorEnum: UIColorToken.Sidebar_Item_Text
                    colorState: UIColorState.Selected
                }
            }

            indicator: UTLabel {
                x: rowItem.leftMargin + rowItem.depth * rowItem.indentation
                text: "▶"
                fontEnum: UIFontToken.Caption_Text
                colorEnum: UIColorToken.Sidebar_Item_Text
                rotation: rowItem.expanded ? 90 : 0
                visible: rowItem.hasChildren
            }

            contentItem: UTLabel {
                // model.nodeType: 0 = Group, 1 = Camera
                text: (model.nodeType === 0 ? "📁 " : "🎥 ") + model.displayName
                fontEnum: UIFontToken.Body_Text
                colorEnum: UIColorToken.Sidebar_Item_Text
                colorState: rowItem.isSelected ? UIColorState.Selected : UIColorState.Normal
            }

            background: Rectangle {
                color: rowItem.isSelected
                    ? UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Background, UIColorState.Selected)
                    : rowItem.hovered
                        ? UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Background, UIColorState.Hovered)
                        : "transparent"
            }

            onClicked: controller.selectNode(nodeId)

            Drag.active: dragHandler.active
            Drag.dragType: Drag.Automatic
            Drag.supportedActions: Qt.MoveAction
            Drag.mimeData: { "text/x-camera-node-id": nodeId }
            Drag.hotSpot.x: 8
            Drag.hotSpot.y: dragPreview.height / 2

            TapHandler {
                acceptedButtons: Qt.LeftButton
                onPressedChanged: {
                    if (pressed) {
                        dragPreview.grabToImage(function(result) {
                            rowItem.Drag.imageSource = result.url;
                        });
                    }
                }
            }

            DragHandler {
                id: dragHandler
                target: null
                dragThreshold: 6
                grabPermissions: PointerHandler.CanTakeOverFromAnything
            }

            UTFocusItem {
                delegateFocused: rowItem.treeView.currentRow === rowItem.row
                                 && rowItem.treeView.activeFocus
            }
        }
    }

    // Drop target for drag-reparent.
    DropArea {
        id: rootDropArea
        anchors.fill: treeContainer

        function targetIdAt(x, y) {
            const tv = treeContainer.treeView;
            if (!tv) return "";
            const p = mapToItem(tv, x, y);
            const cell = tv.cellAtPosition(p.x + tv.contentX, p.y + tv.contentY);
            if (cell.x < 0 || cell.y < 0) return "";
            const idx = tv.modelIndex(cell);
            return tv.model.data(idx, Qt.UserRole + 1) || "";
        }

        onEntered: (drag) => {
            drag.accepted = drag.formats.indexOf("text/x-camera-node-id") >= 0;
        }
        onPositionChanged: (drag) => {
            const srcId = drag.getDataAsString("text/x-camera-node-id");
            const tgtId = targetIdAt(drag.x, drag.y);
            drag.accepted = controller.canDropOnNode(srcId, tgtId);
        }
        onDropped: (drop) => {
            const srcId = drop.getDataAsString("text/x-camera-node-id");
            const tgtId = targetIdAt(drop.x, drop.y);
            if (!controller.canDropOnNode(srcId, tgtId)) {
                drop.accepted = false;
                return;
            }
            controller.moveCameraNode(srcId, tgtId);
            drop.accepted = true;
        }
    }

    // Expand the new parent once the move has been applied to the model.
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

    // Status overlay shown while loading or after a load failure.
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
