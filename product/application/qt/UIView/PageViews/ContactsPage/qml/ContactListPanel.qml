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

        // Right-click: resolve the hit node and open the context menu.
        onContextRequested: function(idx, pos) {
            const id   = idx.valid ? (treeView.model.data(idx, Qt.UserRole + 1) || "") : "";
            const type = idx.valid ? treeView.model.data(idx, Qt.UserRole + 3) : -1;
            contextMenu.openFor(id, type, pos);
        }

        // Drag-reparent: declare what we accept and how to validate.
        acceptedMimeTypes: ["text/x-contact-id"]
        dropValidate: function(_mt, data, idx) {
            const targetId = idx.valid ? (treeView.model.data(idx, Qt.UserRole + 1) || "") : "";
            return controller.canDropOn(data, targetId);
        }
        onDropAccepted: function(_mt, data, idx) {
            const targetId = idx.valid ? (treeView.model.data(idx, Qt.UserRole + 1) || "") : "";
            controller.moveContact(data, targetId);
        }

        delegate: UTTreeViewDelegate {
            text: model.displayName
            highlighted: model.id === controller.currentContactId
            dragMimeData: ({ "text/x-contact-id": model.id })
            onClicked: controller.selectContact(model.id)
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
        // Once a freshly-added node exists in the model, expand its parent and select it.
        function onContactAdded(newId, parentId) {
            const tv = treeContainer.treeView;
            if (!tv) return;
            if (parentId !== "") {
                const pidx = tv.model.indexOfId(parentId);
                const pr = tv.rowAtIndex(pidx);
                if (pr >= 0) tv.expand(pr);
            }
            controller.selectContact(newId);
        }
    }

    // ---- Context menu ----
    // The controller owns the menu model (built from real add/remove permissions) and
    // decides which dialog an action opens. The panel only forwards the user's choice.
    UTDynamicMenu {
        id: contextMenu
        parent: treeContainer

        property string targetId: ""
        property int    targetType: -1   // -1 = blank/root, 0 = Person, 1 = Group

        function openFor(id, type, pos) {
            targetId = id;
            targetType = type;
            model = root.controller.contextMenuModel(id, type);
            popup(pos.x, pos.y);
        }

        onItemTriggered: action => root.controller.handleContextAction(action, targetId, targetType)
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
