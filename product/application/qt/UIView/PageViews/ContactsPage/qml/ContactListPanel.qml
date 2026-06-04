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

            DropArea {
                anchors.fill: parent

                onEntered: (drag) => {
                    if (drag.formats.indexOf("text/x-contact-id") < 0) {
                        drag.accepted = false;
                        return;
                    }
                    const srcId = drag.getDataAsString("text/x-contact-id");
                    drag.accepted = controller.canDropOn(srcId, model.id);
                }
                onDropped: (drop) => {
                    const srcId = drop.getDataAsString("text/x-contact-id");
                    const ok = controller.canDropOn(srcId, model.id);
                    if (!ok) { drop.accepted = false; return; }
                    controller.moveContact(srcId, model.id);
                    drop.accepted = true;
                }
            }
        }
    }

    DropArea {
        id: rootDropArea
        anchors.fill: treeContainer
        z: -1

        // Returns true if the cursor (given drag/drop event) is currently over
        // a real row of the TreeView. We use this to suppress root acceptance
        // so the row's own DropArea handles it instead.
        function isOverRow(drag) {
            const tv = treeContainer.treeView;
            if (!tv) return false;
            const p = mapToItem(tv, drag.x, drag.y);
            const cell = tv.cellAtPosition(p.x + tv.contentX, p.y + tv.contentY);
            return cell.x >= 0 && cell.y >= 0;
        }

        onEntered: (drag) => {
            if (drag.formats.indexOf("text/x-contact-id") < 0) { drag.accepted = false; return; }
            if (isOverRow(drag))                               { drag.accepted = false; return; }
            const srcId = drag.getDataAsString("text/x-contact-id");
            drag.accepted = controller.canDropOn(srcId, "");
        }
        onPositionChanged: (drag) => {
            if (isOverRow(drag)) { drag.accepted = false; return; }
            const srcId = drag.getDataAsString("text/x-contact-id");
            drag.accepted = controller.canDropOn(srcId, "");
        }
        onDropped: (drop) => {
            if (isOverRow(drop)) { drop.accepted = false; return; }
            const srcId = drop.getDataAsString("text/x-contact-id");
            const ok = controller.canDropOn(srcId, "");
            if (!ok) { drop.accepted = false; return; }
            controller.moveContact(srcId, "");
            drop.accepted = true;
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
