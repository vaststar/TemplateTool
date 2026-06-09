import QtQuick
import QtQuick.Controls

FocusScope {
    id: root

    property alias model: treeView.model
    property alias delegate: treeView.delegate
    property alias treeView: treeView
    property bool expandAll: false
    signal itemInvoked(var modelIndex)

    property var acceptedMimeTypes: []
    property var dropValidate: function(_mimeType, _data, _targetIndex) { return false }
    signal dropAccepted(string mimeType, string data, var targetIndex)

    clip: true
    activeFocusOnTab: true

    Component.onCompleted: {
        if (visible) Qt.callLater(forceActiveFocus)
    }
    onVisibleChanged: {
        if (visible) forceActiveFocus()
    }

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

        onWidthChanged: forceLayout()

        Timer {
            interval: 0
            running: root.expandAll && treeView.model !== null
            onTriggered: treeView.expandRecursively()
        }

        onRowsChanged: _ensureCurrentIndex()
        onActiveFocusChanged: { if (activeFocus) _ensureCurrentIndex() }

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
            if (currentRow >= 0) root.itemInvoked(index(currentRow, 0));
        }
    }

    DropArea {
        id: _dropArea
        anchors.fill: treeView
        enabled: root.acceptedMimeTypes.length > 0

        function _pickMime(formats) {
            const accepted = root.acceptedMimeTypes
            for (var i = 0; i < accepted.length; ++i) {
                if (formats.indexOf(accepted[i]) >= 0) return accepted[i]
            }
            return ""
        }
        function _indexAt(x, y) {
            const cell = treeView.cellAtPosition(x + treeView.contentX,
                                                 y + treeView.contentY)
            if (cell.x < 0 || cell.y < 0) return treeView.index(-1, -1)
            return treeView.modelIndex(cell)
        }

        onEntered: (drag) => { drag.accepted = _pickMime(drag.formats) !== "" }
        onPositionChanged: (drag) => {
            const mt = _pickMime(drag.formats)
            if (mt === "") { drag.accepted = false; return }
            drag.accepted = root.dropValidate(mt, drag.getDataAsString(mt),
                                              _indexAt(drag.x, drag.y))
        }
        onDropped: (drop) => {
            const mt = _pickMime(drop.formats)
            if (mt === "") { drop.accepted = false; return }
            const data = drop.getDataAsString(mt)
            const idx  = _indexAt(drop.x, drop.y)
            if (!root.dropValidate(mt, data, idx)) { drop.accepted = false; return }
            root.dropAccepted(mt, data, idx)
            drop.accepted = true
        }
    }
}
