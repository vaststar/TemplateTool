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
 * - Built-in delete confirmation dialog
 * - Customisable thumbnails via Component injection
 * - Auto scroll-to-latest after refresh
 *
 * Usage:
 *   UTFolderView {
 *       folderUrl: "file:///path/to/folder"
 *       nameFilters: ["*.png", "*.jpg"]
 *       onFileOpenRequested: (fp) => myController.openFile(fp)
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
    property string deleteDialogTitle: qsTr("Delete File")
    property string deleteDialogMessage: qsTr("Are you sure you want to delete this file?")

    // ─── Delegate customisation ───
    property Component gridThumbnail: null
    property Component detailIcon: null
    property Component extraMenuItems: null
    property Component statusBarExtra: null

    // ─── Read-only properties ───
    readonly property alias fileCount: folderModel.count
    readonly property string selectedFilePath: _selectedFilePath
    readonly property int selectedIndex: _selectedIndex
    readonly property string viewMode: _viewMode

    // ─── Signals ───
    signal fileOpenRequested(string filePath)
    signal fileCopyRequested(string filePath)
    signal fileRevealRequested(string filePath)
    signal fileDeleteRequested(string filePath)
    signal fileSelected(string filePath)
    signal fileDeselected()

    // ─── Public methods ───
    function refresh() {
        folderModel.folder = ""
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
        _selectedIndex = -1
        _selectedFilePath = ""
        fileDeselected()
    }

    // ─── Internal state ───
    property string _viewMode: initialViewMode
    property int _selectedIndex: -1
    property string _selectedFilePath: ""
    property string _pendingFilePath: ""
    property bool   _pendingLatest: false

    Timer {
        id: _refreshTimer
        interval: 100
        repeat: false
        onTriggered: folderModel.folder = control.folderUrl
    }

    Connections {
        target: folderModel
        function onStatusChanged() {
            if ((control._pendingFilePath !== "" || control._pendingLatest)
                && folderModel.status === FolderListModel.Ready) {
                Qt.callLater(_doScrollPending)
            }
        }
    }

    function _selectItem(index) {
        _selectedIndex = index
        _selectedFilePath = folderModel.get(index, "filePath")
        fileSelected(_selectedFilePath)
    }

    function _openContextMenuAtItem(index, view) {
        _selectItem(index)
        _contextMenu.currentFilePath = _selectedFilePath
        var item = view.itemAtIndex(index)
        if (item) {
            var pos = item.mapToItem(control, item.width, 0)
            _contextMenu.popup(control, pos)
        } else {
            _contextMenu.popup()
        }
    }

    function _doScrollPending() {
        if (folderModel.count <= 0) return

        var targetIndex = -1

        if (_pendingFilePath !== "") {
            targetIndex = _findIndexByFilePath(_pendingFilePath)
            _pendingFilePath = ""
        } else if (_pendingLatest) {
            targetIndex = _findLatestIndex()
            _pendingLatest = false
        }

        if (targetIndex < 0) return
        _selectItem(targetIndex)
        _scrollViewToIndex(targetIndex)
    }

    function _findIndexByFilePath(path) {
        var needle = _toLocalPath(path)
        for (var i = 0; i < folderModel.count; i++) {
            if (_toLocalPath(folderModel.get(i, "filePath")) === needle)
                return i
        }
        return -1
    }

    // Strip file:// scheme, decode percent-encoding, normalise slashes & case
    function _toLocalPath(p) {
        var s = decodeURIComponent(String(p))
        if (s.startsWith("file:///"))
            s = s.substring(8)          // file:///C:/foo → C:/foo
        else if (s.startsWith("file://"))
            s = s.substring(7)          // file:///foo (Unix) → /foo
        return s.replace(/\\/g, "/").toLowerCase()
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
            _selectItem(index)
            _contextMenu.currentFilePath = _selectedFilePath
            _contextMenu.popup()
        } else {
            if (_selectedIndex === index) clearSelection()
            else _selectItem(index)
        }
    }

    // ─── FolderListModel ───
    FolderListModel {
        id: folderModel
        folder: control.folderUrl
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
                selectedIndex: control._selectedIndex

                onItemInvoked: function(index) {
                    control._openContextMenuAtItem(index, _gridView.gridView)
                }

                delegate: UTFolderGridDelegate {
                    width: _gridView.cellWidth
                    height: _gridView.cellHeight
                    thumbnailComponent: control.gridThumbnail
                    isSelected: control._selectedIndex === index
                    isCurrent: _gridView.gridView.currentIndex === index

                    onClicked: function(mouseButton) {
                        _gridView.focusOnItem(index)
                        control._handleItemClick(index, mouseButton)
                    }
                    onDoubleClicked: {
                        _gridView.focusOnItem(index)
                        control.fileOpenRequested(filePath)
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
                selectedIndex: control._selectedIndex

                onItemInvoked: function(index) {
                    control._openContextMenuAtItem(index, _detailContainer.listView)
                }

                delegate: UTFolderDetailDelegate {
                    width: _detailContainer.contentAvailableWidth
                    iconComponent: control.detailIcon
                    isSelected: control._selectedIndex === index
                    isCurrent: _detailContainer.listView.currentIndex === index

                    onClicked: function(mouseButton) {
                        _detailContainer.focusOnItem(index)
                        control._handleItemClick(index, mouseButton)
                    }
                    onDoubleClicked: {
                        _detailContainer.focusOnItem(index)
                        control.fileOpenRequested(filePath)
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
            onTriggered: control.fileOpenRequested(_contextMenu.currentFilePath)
        }
        UTMenuItem {
            text: qsTr("Copy to Clipboard")
            onTriggered: control.fileCopyRequested(_contextMenu.currentFilePath)
        }
        UTMenuItem {
            text: qsTr("Show in Explorer")
            onTriggered: control.fileRevealRequested(_contextMenu.currentFilePath)
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
                _deleteDialog.targetFilePath = _contextMenu.currentFilePath
                _deleteDialog.open()
            }
        }
    }

    // ─── Delete confirmation ───
    Dialog {
        id: _deleteDialog
        property string targetFilePath: ""
        title: control.deleteDialogTitle
        modal: true
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: Math.min(400, control.width * 0.8)
        standardButtons: Dialog.Yes | Dialog.No

        contentItem: UTText {
            text: control.deleteDialogMessage
            fontEnum: UIFontToken.Body_Text
        }

        onAccepted: {
            control.fileDeleteRequested(targetFilePath)
            control.clearSelection()
            control.refresh()
        }
    }
}
