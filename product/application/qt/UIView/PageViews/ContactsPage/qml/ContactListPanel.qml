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

        delegate: TreeViewDelegate {
            id: rowItem
            implicitWidth: treeView.width
            indentation: 16
            leftMargin: 8
            spacing: 6
            topPadding: 6
            bottomPadding: 6
            focusPolicy: Qt.NoFocus
            z: (treeView.currentRow === row && treeView.activeFocus) ? 1 : 0

            readonly property string nodeId: model.id
            readonly property bool isSelected: nodeId === controller.currentContactId

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
                    text: model.displayName
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
                text: model.displayName
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

            onClicked: controller.selectContact(nodeId)

            Drag.active: dragHandler.active
            Drag.dragType: Drag.Automatic
            Drag.supportedActions: Qt.MoveAction
            Drag.mimeData: { "text/x-contact-id": nodeId }
            Drag.hotSpot.x: 12
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
