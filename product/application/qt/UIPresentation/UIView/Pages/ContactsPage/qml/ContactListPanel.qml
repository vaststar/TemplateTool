import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: root

    required property ContactsPageController controller

    function clearSearch(refocus) {
        searchField.text = "";
        searchDelay.stop();
        controller.searchText = "";
        if (refocus) searchField.forceActiveFocus();
    }

    Timer {
        id: searchDelay
        interval: 150
        repeat: false
        onTriggered: root.controller.searchText = searchField.text
    }

    Shortcut {
        sequence: StandardKey.Find
        enabled: root.visible && searchField.enabled
        onActivated: {
            searchField.forceActiveFocus();
            searchField.selectAll();
        }
    }

    // Panel background – slightly different from content area
    Rectangle {
        anchors.fill: parent
        color: UTComponentUtil.getPlainUIColor(UIColorToken.Main_Window_Background, UIColorState.Normal)
    }

    // Header + name search
    Rectangle {
        id: header
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        height: 80
        color: "transparent"

        ColumnLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 8
            anchors.topMargin: 6
            anchors.bottomMargin: 6
            spacing: 6

            UTText {
                Layout.fillWidth: true
                Layout.preferredHeight: 28
                text: qsTr("联系人列表")
                fontEnum: UIFontToken.Section_Title
                colorEnum: UIColorToken.Sidebar_Item_Text
                verticalAlignment: Text.AlignVCenter
            }

            UTTextField {
                id: searchField
                Layout.fillWidth: true
                Layout.preferredHeight: 30
                placeholderText: qsTr("搜索联系人")
                rightPadding: clearButton.visible ? 30 : 8
                enabled: root.controller
                         && root.controller.loadState === ContactsPageController.Ready
                onTextChanged: searchDelay.restart()

                Keys.onEscapePressed: function(event) {
                    if (text.length === 0) return;
                    root.clearSearch(true);
                    event.accepted = true;
                }

                UTButton {
                    id: clearButton
                    anchors.right: parent.right
                    anchors.rightMargin: 4
                    anchors.verticalCenter: parent.verticalCenter
                    width: 22
                    height: 22
                    radius: 4
                    borderWidth: 0
                    visible: searchField.text.length > 0
                    text: "×"
                    backgroundColorEnum: UIColorToken.Content_Input_Background
                    fontColorEnum: UIColorToken.Content_Input_Text
                    onClicked: root.clearSearch(true)
                }
            }
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
            if (searchField.text.trim().length !== 0) return false;
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
            dragMimeData: searchField.text.trim().length === 0
                          ? ({ "text/x-contact-id": model.id })
                          : null
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
            root.clearSearch(false);
            Qt.callLater(function() {
                const tv = treeContainer.treeView;
                if (!tv) return;
                if (parentId !== "") {
                    const pidx = tv.model.indexOfId(parentId);
                    const pr = tv.rowAtIndex(pidx);
                    if (pr >= 0) tv.expand(pr);
                }
                controller.selectContact(newId);
            });
        }
        function onSearchTextChanged() {
            Qt.callLater(function() {
                const tv = treeContainer.treeView;
                if (tv && tv.rows > 0) tv.expandRecursively();
            });
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

    // Ready, but the name filter removed every root branch.
    Item {
        anchors {
            left: parent.left
            right: parent.right
            top: header.bottom
            bottom: parent.bottom
        }
        visible: treeContainer.visible
                 && root.controller.searchText.length > 0
                 && treeContainer.treeView.rows === 0

        UTText {
            anchors.centerIn: parent
            text: qsTr("没有找到匹配的联系人")
            fontEnum: UIFontToken.Body_Text
            colorEnum: UIColorToken.Sidebar_Item_Text
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
