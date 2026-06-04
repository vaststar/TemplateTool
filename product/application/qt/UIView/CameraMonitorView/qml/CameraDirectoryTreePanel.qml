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

        delegate: Item {
            id: rowItem
            implicitWidth: treeView.width
            implicitHeight: label.implicitHeight * 1.8
            z: (current && treeView.activeFocus) ? 1 : 0

            readonly property real indentation: 16
            readonly property real padding: 8

            required property TreeView treeView
            required property bool isTreeNode
            required property bool expanded
            required property bool hasChildren
            required property int depth
            required property int row
            required property int column
            required property bool current

            // ----- Drag source -----
            Drag.active: dragHandler.active
            Drag.dragType: Drag.Automatic
            Drag.supportedActions: Qt.MoveAction
            Drag.mimeData: { "text/x-camera-node-id": model.id }
            Drag.hotSpot.x: 8
            Drag.hotSpot.y: 8

            TapHandler {
                id: pressTap
                acceptedButtons: Qt.LeftButton
                onPressedChanged: {
                    if (pressed) {
                        rowItem.grabToImage(function(result) {
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

            property Animation indicatorAnimation: NumberAnimation {
                target: indicator
                property: "rotation"
                from: expanded ? 0 : 90
                to: expanded ? 90 : 0
                duration: 100
                easing.type: Easing.OutQuart
            }
            TableView.onPooled: indicatorAnimation.complete()
            TableView.onReused: if (current) indicatorAnimation.start()
            onExpandedChanged: indicator.rotation = expanded ? 90 : 0

            HoverHandler { id: hoverHandler }

            Rectangle {
                id: background
                anchors.fill: parent
                color: model.id === controller.currentCameraId
                    ? UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Background, UIColorState.Selected)
                    : hoverHandler.hovered
                        ? UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Background, UIColorState.Hovered)
                        : "transparent"
            }

            UTLabel {
                id: indicator
                x: padding + (depth * indentation)
                anchors.verticalCenter: parent.verticalCenter
                visible: isTreeNode && hasChildren
                text: "▶"
                fontEnum: UIFontToken.Caption_Text
                colorEnum: UIColorToken.Sidebar_Item_Text

                TapHandler {
                    onSingleTapped: {
                        const idx = treeView.index(row, column);
                        treeView.selectionModel.setCurrentIndex(idx, ItemSelectionModel.NoUpdate);
                        treeView.toggleExpanded(row);
                    }
                }
            }

            UTLabel {
                id: label
                x: padding + (isTreeNode ? (depth + 1) * indentation : 0)
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - padding - x
                // model.nodeType: 0 = Group, 1 = Camera
                text: (model.nodeType === 0 ? "📁 " : "🎥 ") + model.displayName
                fontEnum: UIFontToken.Body_Text
                colorEnum: UIColorToken.Sidebar_Item_Text
                colorState: model.id === controller.currentCameraId ? UIColorState.Selected : UIColorState.Normal
            }

            TapHandler {
                onSingleTapped: {
                    const idx = treeView.index(row, column);
                    treeView.selectionModel.setCurrentIndex(idx, ItemSelectionModel.NoUpdate);
                    controller.selectNode(model.id);
                }
            }

            UTFocusItem {
                delegateFocused: current && treeContainer.treeView.activeFocus
            }

            // Per-row drop target: drop onto this node to make it the new parent.
            DropArea {
                anchors.fill: parent

                onEntered: (drag) => {
                    if (drag.formats.indexOf("text/x-camera-node-id") < 0) {
                        drag.accepted = false;
                        return;
                    }
                    const srcId = drag.getDataAsString("text/x-camera-node-id");
                    drag.accepted = controller.canDropOnNode(srcId, model.id);
                }
                onDropped: (drop) => {
                    const srcId = drop.getDataAsString("text/x-camera-node-id");
                    const ok = controller.canDropOnNode(srcId, model.id);
                    if (!ok) { drop.accepted = false; return; }
                    controller.moveCameraNode(srcId, model.id);
                    // Expand the drop target so the user can immediately see the
                    // newly-arrived child instead of having to click the chevron.
                    if (!expanded) {
                        treeView.expand(row);
                    }
                    drop.accepted = true;
                }
            }
        }
    }

    // Root-level drop target: dropping in empty area below / between rows moves to root.
    DropArea {
        id: rootDropArea
        anchors.fill: treeContainer
        z: -1

        // Returns true if the cursor (given drag/drop event) is currently over a real
        // row of the TreeView. We use this to suppress root acceptance so the row's own
        // DropArea handles it instead.
        function isOverRow(drag) {
            const tv = treeContainer.treeView;
            if (!tv) return false;
            const p = mapToItem(tv, drag.x, drag.y);
            const cell = tv.cellAtPosition(p.x + tv.contentX, p.y + tv.contentY);
            return cell.x >= 0 && cell.y >= 0;
        }

        onEntered: (drag) => {
            if (drag.formats.indexOf("text/x-camera-node-id") < 0) { drag.accepted = false; return; }
            if (isOverRow(drag))                                   { drag.accepted = false; return; }
            const srcId = drag.getDataAsString("text/x-camera-node-id");
            drag.accepted = controller.canDropOnNode(srcId, "");
        }
        onPositionChanged: (drag) => {
            if (isOverRow(drag)) { drag.accepted = false; return; }
            const srcId = drag.getDataAsString("text/x-camera-node-id");
            drag.accepted = controller.canDropOnNode(srcId, "");
        }
        onDropped: (drop) => {
            if (isOverRow(drop)) { drop.accepted = false; return; }
            const srcId = drop.getDataAsString("text/x-camera-node-id");
            const ok = controller.canDropOnNode(srcId, "");
            if (!ok) { drop.accepted = false; return; }
            controller.moveCameraNode(srcId, "");
            drop.accepted = true;
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
