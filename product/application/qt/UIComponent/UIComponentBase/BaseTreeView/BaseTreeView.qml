import QtQuick
import QtQuick.Controls

FocusScope {
    id: root

    property alias model: treeView.model
    property alias delegate: treeView.delegate
    property alias treeView: treeView
    property bool expandAll: false
    signal itemInvoked(var modelIndex)

    clip: true
    activeFocusOnTab: true

    Component.onCompleted: {
        if (visible) Qt.callLater(forceActiveFocus)
    }
    onVisibleChanged: {
        if (visible) forceActiveFocus()
    }

    // Clicking anywhere in the tree should give it keyboard focus
    TapHandler {
        onTapped: treeView.forceActiveFocus()
    }

    TreeView {
        id: treeView
        anchors.fill: parent
        anchors.margins: 4
        clip: false
        activeFocusOnTab: false
        focus: true
        animate: false
        selectionModel: ItemSelectionModel {}

        Timer {
            interval: 0
            running: root.expandAll && treeView.model !== null
            onTriggered: treeView.expandRecursively()
        }

        // When rows are actually laid out, ensure first row is current
        onRowsChanged: _ensureCurrentIndex()

        onActiveFocusChanged: {
            if (activeFocus) _ensureCurrentIndex();
        }

        function _ensureCurrentIndex() {
            if (currentRow < 0 && rows > 0) {
                selectionModel.setCurrentIndex(
                    index(0, 0), ItemSelectionModel.NoUpdate);
            }
        }

        Keys.onReturnPressed: function(e) { e.accepted = true; _emitActivated(); }
        Keys.onEnterPressed:  function(e) { e.accepted = true; _emitActivated(); }
        Keys.onSpacePressed:  function(e) { e.accepted = true; _emitActivated(); }

        function _emitActivated() {
            if (currentRow >= 0) {
                root.itemInvoked(index(currentRow, 0));
            }
        }
    }
}
