import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.folderlistmodel
import UIComponentBase 1.0
import UTComponent 1.0
import UTComposite 1.0
import UIResourceLoader 1.0

/**
 * UTFolderView - Reusable folder browser with Grid / List / Detail views.
 *
 * Features:
 * - Two view modes: grid (tiles), detail (table)
 * - Built-in header with view-mode toggle
 * - Built-in context menu: Open, Copy, Reveal, Delete
 * - Customisable thumbnails via Component injection
 * - Auto scroll-to-latest after refresh
 *
 * Usage:
 *   UTFolderView {
 *       folderUrl: "file:///path/to/folder"
 *       nameFilters: ["*.png", "*.jpg"]
 *       onFileOpenRequested: (fp) => myController.openFile(fp)
 *       onFileDeleteRequested: (fp) => myController.deleteFile(fp)
 *   }
 */
BaseFolderView {
    id: control

    // ─── Required ───
    required property url folderUrl
    property var nameFilters: []

    // ─── Configuration ───
    property string initialViewMode: "grid"          // "grid" | "detail"

    property string emptyIcon: "📁"
    property string emptyTitle: qsTr("No files")
    property string emptyHint: ""
    property string statusTemplate: qsTr("%1 files")

    // ─── Delegate customisation ───
    property Component gridThumbnail: null
    property Component detailIcon: null
    property Component extraMenuItems: null
    property Component statusBarExtra: null

    // ─── Read-only properties ───
    readonly property alias fileCount: folderModel.count
    readonly property string selectedFilePath: _selectedIndex >= 0 ? _selectedFilePath : ""
    readonly property int selectedIndex: _selectedIndex
    readonly property string viewMode: _viewMode

    // ─── Signals ───
    signal fileOpenRequested(string filePath)
    signal fileCopyRequested(string filePath)
    signal fileRevealRequested(string filePath)
    // Emitted when the user chooses Delete. The host must confirm and delete;
    // this view performs no deletion or confirmation itself.
    signal fileDeleteRequested(string filePath)
    signal fileSelected(string filePath)
    signal fileDeselected()

    // ─── Public methods ───
    function refresh() {
        _awaitingLoadStart = true
        _invalidateSelection()
        _refreshSuspended = true
        _refreshTimer.restart()
    }

    function scrollToFile(filePath) {
        _pendingFilePath = filePath
        _pendingLatest = false
        refresh()
    }

    function scrollToLatest() {
        _pendingLatest = true
        _pendingFilePath = ""
        refresh()
    }

    function clearSelection() {
        _invalidateSelection()
        _selectedFilePath = ""
    }

    // ─── Internal state ───
    property string _viewMode: initialViewMode
    property int _selectedIndex: -1
    // Retain the path while reloading; expose it only after validation.
    property string _selectedFilePath: ""
    property string _pendingFilePath: ""
    property bool   _pendingLatest: false
    property bool   _refreshSuspended: false
    property bool   _awaitingLoadStart: true

    onFolderUrlChanged: {
        _awaitingLoadStart = true
        _pendingFilePath = ""
        _pendingLatest = false
        clearSelection()
        if (_refreshTimer) _refreshTimer.stop()
        _refreshSuspended = false
    }

    Timer {
        id: _refreshTimer
        interval: 100
        repeat: false
        onTriggered: control._refreshSuspended = false
    }

    Connections {
        target: folderModel

        function onModelAboutToBeReset() {
            control._awaitingLoadStart = true
            control._invalidateSelection()
        }
        function onFolderChanged() {
            // A full folder result can also release the wait for a fresh load.
            control._awaitingLoadStart = false
            control._scheduleSelectionSync()
        }
        function onStatusChanged() {
            if (folderModel.status === FolderListModel.Loading
                && !control._refreshSuspended) {
                control._awaitingLoadStart = false
            }
            if (folderModel.status !== FolderListModel.Ready)
                control._invalidateSelection()
            control._scheduleSelectionSync()
        }
        function onRowsAboutToBeInserted() { control._invalidateSelection() }
        function onRowsAboutToBeRemoved() { control._invalidateSelection() }
        function onLayoutAboutToBeChanged() { control._invalidateSelection() }
        function onModelReset() { control._scheduleSelectionSync() }
        function onRowsInserted() { control._scheduleSelectionSync() }
        function onRowsRemoved() { control._scheduleSelectionSync() }
        function onLayoutChanged() { control._scheduleSelectionSync() }
        function onDataChanged() {
            control._invalidateSelection()
            control._scheduleSelectionSync()
        }
    }

    function _invalidateSelection() {
        var hadSelection = _selectedIndex >= 0
        _selectedIndex = -1
        if (_contextMenu) {
            _contextMenu.currentFilePath = ""
            _contextMenu.close()
        }
        if (hadSelection) fileDeselected()
    }

    function _isModelReady() {
        return !_refreshSuspended && !_awaitingLoadStart
            && folderModel.status === FolderListModel.Ready
            && String(folderModel.folder) === String(folderUrl)
    }

    function _scheduleSelectionSync() {
        // Coalesce model signals and read the final state, not a captured index.
        Qt.callLater(_reconcileSelection)
    }

    function _selectItem(index) {
        if (!_isModelReady() || index < 0 || index >= folderModel.count)
            return false
        var path = folderModel.get(index, "filePath")
        if (!path) return false
        var changed = _selectedIndex !== index || _selectedFilePath !== path
        _selectedFilePath = path
        _selectedIndex = index
        if (changed) fileSelected(path)
        return true
    }

    function _openContextMenuAtItem(index, view) {
        if (!_selectItem(index)) return
        _contextMenu.currentFilePath = selectedFilePath
        var item = view.itemAtIndex(index)
        if (item) {
            var pos = item.mapToItem(control, item.width, 0)
            _contextMenu.popup(control, pos)
        } else {
            _contextMenu.popup()
        }
    }

    function _reconcileSelection() {
        if (!_isModelReady()) return

        var targetIndex = -1
        if (_pendingFilePath !== "") {
            targetIndex = _findIndexByFilePath(_pendingFilePath)
        } else if (_pendingLatest && folderModel.count > 0) {
            targetIndex = _findLatestIndex()
        }
        var scrollToTarget = targetIndex >= 0
        // Consume the request even when the completed list is empty.
        _pendingFilePath = ""
        _pendingLatest = false

        if (targetIndex < 0 && _selectedFilePath !== "")
            targetIndex = _findIndexByFilePath(_selectedFilePath)

        if (targetIndex < 0) {
            clearSelection()
            return
        }
        if (_selectItem(targetIndex) && scrollToTarget)
            _scrollViewToIndex(targetIndex)
    }

    function _findIndexByFilePath(path) {
        var needle = String(path)
        if (Qt.platform.os === "windows") needle = needle.replace(/\\/g, "/")
        for (var i = 0; i < folderModel.count; i++) {
            var candidate = String(folderModel.get(i, "filePath"))
            if (Qt.platform.os === "windows") candidate = candidate.replace(/\\/g, "/")
            if (candidate === needle) return i
        }
        // Let Qt parse explicit URLs; native paths must not be percent-decoded.
        return /^file:/i.test(String(path)) ? folderModel.indexOf(path) : -1
    }

    function _findLatestIndex() {
        var best = 0
        var bestTime = folderModel.get(0, "fileModified")
        for (var i = 1; i < folderModel.count; i++) {
            var t = folderModel.get(i, "fileModified")
            if (t > bestTime) { bestTime = t; best = i }
        }
        return best
    }

    function _scrollViewToIndex(index) {
        if (_viewMode === "grid")
            _gridView.gridView.positionViewAtIndex(index, GridView.Beginning)
        else
            _detailContainer.listView.positionViewAtIndex(index, ListView.Beginning)
    }

    function _handleItemClick(index, mouseButton) {
        if (mouseButton === Qt.RightButton) {
            if (!_selectItem(index)) return
            _contextMenu.currentFilePath = selectedFilePath
            _contextMenu.popup()
        } else {
            if (selectedIndex === index) clearSelection()
            else _selectItem(index)
        }
    }

    // ─── FolderListModel ───
    FolderListModel {
        id: folderModel
        folder: control._refreshSuspended ? "" : control.folderUrl
        nameFilters: control.nameFilters
        showDirs: false
        sortField: FolderListModel.Time
        sortReversed: false   // newest first
    }

    // ─── Layout ───
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Header
        UTFolderViewHeader {
            id: _header
            Layout.fillWidth: true
            viewMode: control._viewMode
            scrollBarWidth: _detailContainer.listView.width - _detailContainer.contentAvailableWidth
            onViewModeChangeRequested: function(mode) { control._viewMode = mode }
        }

        // Gallery container
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Background, UIColorState.Normal)
            radius: 8

            // Empty state
            ColumnLayout {
                anchors.centerIn: parent
                visible: folderModel.count === 0
                spacing: 16

                UTText {
                    Layout.alignment: Qt.AlignHCenter
                    text: control.emptyIcon
                    // emoji glyph size, intentionally not tokenized
                    font.pixelSize: 48
                }
                UTText {
                    Layout.alignment: Qt.AlignHCenter
                    text: control.emptyTitle
                    fontEnum: UIFontToken.Body_Text
                    colorEnum: UIColorToken.Content_Secondary_Text
                }
                UTText {
                    Layout.alignment: Qt.AlignHCenter
                    text: control.emptyHint
                    visible: control.emptyHint.length > 0
                    fontEnum: UIFontToken.Caption_Text
                    colorEnum: UIColorToken.Content_Secondary_Text
                }
            }

            // === Grid View ===
            UTGridView {
                id: _gridView
                anchors.fill: parent
                visible: control._viewMode === "grid" && folderModel.count > 0
                topMargin: 8
                bottomMargin: 8
                leftMargin: 8
                rightMargin: 0
                cellWidth: 180
                cellHeight: 160
                model: folderModel
                selectedIndex: control.selectedIndex

                onItemInvoked: function(index) {
                    control._openContextMenuAtItem(index, _gridView.gridView)
                }

                delegate: UTFolderGridDelegate {
                    width: _gridView.cellWidth
                    height: _gridView.cellHeight
                    thumbnailComponent: control.gridThumbnail
                    isSelected: control.selectedIndex === index
                    isCurrent: _gridView.gridView.currentIndex === index

                    onClicked: function(mouseButton) {
                        _gridView.focusOnItem(index)
                        control._handleItemClick(index, mouseButton)
                    }
                    onDoubleClicked: {
                        _gridView.focusOnItem(index)
                        if (control._isModelReady()) control.fileOpenRequested(filePath)
                    }
                }
            }

            // === Detail View ===
            UTListView {
                id: _detailContainer
                anchors.fill: parent
                visible: control._viewMode === "detail" && folderModel.count > 0
                rightMargin: 0
                spacing: 1
                model: folderModel
                selectedIndex: control.selectedIndex

                onItemInvoked: function(index) {
                    control._openContextMenuAtItem(index, _detailContainer.listView)
                }

                delegate: UTFolderDetailDelegate {
                    width: _detailContainer.contentAvailableWidth
                    iconComponent: control.detailIcon
                    isSelected: control.selectedIndex === index
                    isCurrent: _detailContainer.listView.currentIndex === index

                    onClicked: function(mouseButton) {
                        _detailContainer.focusOnItem(index)
                        control._handleItemClick(index, mouseButton)
                    }
                    onDoubleClicked: {
                        _detailContainer.focusOnItem(index)
                        if (control._isModelReady()) control.fileOpenRequested(filePath)
                    }
                }
            }
        }

        // Status bar
        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 6
            Layout.bottomMargin: 4
            Layout.rightMargin: 4
            spacing: 12

            UTText {
                text: control.statusTemplate.arg(folderModel.count)
                fontEnum: UIFontToken.Caption_Text
                colorEnum: UIColorToken.Content_Secondary_Text
            }

            Item { Layout.fillWidth: true }

            Loader {
                active: control.statusBarExtra !== null
                sourceComponent: control.statusBarExtra
            }
        }
    }

    // ─── Context Menu ───
    UTMenu {
        id: _contextMenu
        property string currentFilePath: ""

        UTMenuItem {
            text: qsTr("Open")
            onTriggered: {
                if (_contextMenu.currentFilePath !== "")
                    control.fileOpenRequested(_contextMenu.currentFilePath)
            }
        }
        UTMenuItem {
            text: qsTr("Copy to Clipboard")
            onTriggered: {
                if (_contextMenu.currentFilePath !== "")
                    control.fileCopyRequested(_contextMenu.currentFilePath)
            }
        }
        UTMenuItem {
            text: qsTr("Show in Explorer")
            onTriggered: {
                if (_contextMenu.currentFilePath !== "")
                    control.fileRevealRequested(_contextMenu.currentFilePath)
            }
        }

        UTMenuSeparator {
            visible: _extraMenuLoader.item !== null
        }
        Loader {
            id: _extraMenuLoader
            active: control.extraMenuItems !== null
            sourceComponent: control.extraMenuItems
        }

        UTMenuSeparator {}

        UTMenuItem {
            text: qsTr("Delete")
            onTriggered: {
                if (_contextMenu.currentFilePath !== "")
                    control.fileDeleteRequested(_contextMenu.currentFilePath)
            }
        }
    }
}
