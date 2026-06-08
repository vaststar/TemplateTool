import QtQuick
import QtQuick.Controls
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: root

    required property ContactsPageController controller

    // Panel background – slightly different from content area
    Rectangle {
        anchors.fill: parent
        color: UTComponentUtil.getPlainUIColor(UIColorToken.Main_Window_Background, UIColorState.Normal)
    }

    // Header
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
            text: "联系人列表"
            fontEnum: UIFontToken.Body_Text_Medium
            colorEnum: UIColorToken.Sidebar_Item_Text
            font.bold: true
        }
    }

    // Tree container
    UTTreeView {
        id: treeContainer
        anchors {
            left: parent.left
            right: parent.right
            top: header.bottom
            bottom: parent.bottom
        }
        visible: controller && controller.loadState === ContactsPageController.Ready
                 && controller.orgTreeModel !== null
        model: controller ? controller.orgTreeModel : null
        expandAll: true

        onItemInvoked: function(idx) {
            var contactId = treeView.model.data(idx, Qt.UserRole + 1);
            if (contactId) controller.selectContact(contactId);
        }

        delegate: Item {
            id: rowItem
            implicitWidth: treeView.width
            implicitHeight: label.implicitHeight * 1.8
            z: (current && treeView.activeFocus) ? 1 : 0

            readonly property real indentation: 16
            readonly property real padding: 8

            // Assigned to by TreeView:
            required property TreeView treeView
            required property bool isTreeNode
            required property bool expanded
            required property bool hasChildren
            required property int depth
            required property int row
            required property int column
            required property bool current

            Drag.active: dragHandler.active
            Drag.dragType: Drag.Automatic
            Drag.supportedActions: Qt.MoveAction
            Drag.mimeData: { "text/x-contact-id": model.id }
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

            HoverHandler { id: hoverHandler }

            Rectangle {
                id: background
                anchors.fill: parent
                color: model.id === controller.currentContactId
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
                rotation: expanded ? 90 : 0

                TapHandler {
                    onSingleTapped: {
                        const idx = treeView.index(row, column);
                        treeView.selectionModel.setCurrentIndex(idx, ItemSelectionModel.NoUpdate);
                        controller.selectContact(model.id);
                        treeView.toggleExpanded(row);
                    }
                }
            }

            UTLabel {
                id: label
                x: padding + (isTreeNode ? (depth + 1) * indentation : 0)
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - padding - x
                text: model.displayName
                fontEnum: UIFontToken.Body_Text
                colorEnum: UIColorToken.Sidebar_Item_Text
                colorState: model.id === controller.currentContactId ? UIColorState.Selected : UIColorState.Normal
            }

            // Click to select
            TapHandler {
                onSingleTapped: {
                    const idx = treeView.index(row, column);
                    treeView.selectionModel.setCurrentIndex(idx, ItemSelectionModel.NoUpdate);
                    controller.selectContact(model.id);
                }
            }

            UTFocusItem {
                delegateFocused: current && treeContainer.treeView.activeFocus
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
            drag.accepted = drag.formats.indexOf("text/x-contact-id") >= 0;
        }
        onPositionChanged: (drag) => {
            const srcId = drag.getDataAsString("text/x-contact-id");
            drag.accepted = controller.canDropOn(srcId, targetIdAt(drag.x, drag.y));
        }
        onDropped: (drop) => {
            const srcId = drop.getDataAsString("text/x-contact-id");
            const tgtId = targetIdAt(drop.x, drop.y);
            if (!controller.canDropOn(srcId, tgtId)) {
                drop.accepted = false;
                return;
            }
            controller.moveContact(srcId, tgtId);
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

    // Loading / error overlay (shown when tree is not Ready)
    Item {
        anchors {
            left: parent.left
            right: parent.right
            top: header.bottom
            bottom: parent.bottom
        }
        visible: !treeContainer.visible

        UTText {
            anchors.centerIn: parent
            text: {
                if (!controller) return "";
                if (controller.loadState === ContactsPageController.Error) return "加载失败";
                return "加载中…";
            }
            fontEnum: UIFontToken.Body_Text
            colorEnum: UIColorToken.Sidebar_Item_Text
        }
    }
}
