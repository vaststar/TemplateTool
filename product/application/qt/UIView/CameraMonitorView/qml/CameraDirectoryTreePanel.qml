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
        }
    }

    // Single unified drop target: hit-test the cursor position to find the
    // target node id (or "" for the virtual root). Doing everything in one
    // DropArea avoids the Qt behaviour where rejecting in onEntered
    // permanently deactivates the area for the rest of the drag session
    // (which used to cause root drops to need a window-exit-and-re-enter).
    DropArea {
        id: rootDropArea
        anchors.fill: treeContainer

        // "" means root drop.
        function targetIdAt(x, y) {
            const tv = treeContainer.treeView;
            if (!tv) return "";
            const p = mapToItem(tv, x, y);
            const cell = tv.cellAtPosition(p.x + tv.contentX, p.y + tv.contentY);
            if (cell.x < 0 || cell.y < 0) return "";
            const idx = tv.modelIndex(cell);
            return tv.model.data(idx, Qt.UserRole + 1) || "";
        }
        function cellAt(x, y) {
            const tv = treeContainer.treeView;
            if (!tv) return Qt.point(-1, -1);
            const p = mapToItem(tv, x, y);
            return tv.cellAtPosition(p.x + tv.contentX, p.y + tv.contentY);
        }

        // NOTE: must always accept here as long as the mime type matches.
        // If we reject in onEntered (e.g. when the cursor starts on top of
        // the dragged row itself, making target == source), Qt deactivates
        // this DropArea for the rest of the drag and onPositionChanged
        // will never fire again. The real accept state is computed in
        // onPositionChanged on every frame.
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
            // Expand the drop target so the user can immediately see the
            // newly-arrived child instead of having to click the chevron.
            if (tgtId !== "") {
                const cell = cellAt(drop.x, drop.y);
                if (cell.y >= 0) treeContainer.treeView.expand(cell.y);
            }
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
