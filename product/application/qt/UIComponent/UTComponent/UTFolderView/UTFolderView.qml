import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.folderlistmodel
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTFolderView - Reusable folder browser with Grid / List / Detail views.
 *
 * Features:
 * - Three view modes: grid (tiles), list (rows), detail (table)
 * - Built-in header with sorting and view-mode toggle
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
    property url folderUrl
    property var nameFilters: []

    // ─── Configuration ───
    property string initialViewMode: "grid"          // "grid" | "list" | "detail"
    property int initialSortField: FolderListModel.Time
    property bool initialSortAscending: true

    property string emptyIcon: "📁"
    property string emptyTitle: qsTr("No files")
    property string emptyHint: ""
    property string statusTemplate: qsTr("%1 files")
    property string deleteDialogTitle: qsTr("Delete File")
    property string deleteDialogMessage: qsTr("Are you sure you want to delete this file?")

    // ─── Delegate customisation ───
    property Component gridThumbnail: null
    property Component listThumbnail: null
    property Component detailIcon: null
    property Component extraMenuItems: null
    property Component statusBarExtra: null

    // ─── Read-only properties ───
    readonly property alias fileCount: folderModel.count
    readonly property string selectedFilePath: _selectedFilePath
    readonly property int selectedIndex: _selectedIndex
    readonly property string viewMode: _viewMode
    readonly property int currentSortField: folderModel.sortField
    readonly property bool currentSortAscending: _sortAscending

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

    function scrollToLatest() {
        _scrollPending = true
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
    property bool _scrollPending: false
    property bool _sortAscending: initialSortAscending

    Timer {
        id: _refreshTimer
        interval: 100
        repeat: false
        onTriggered: folderModel.folder = control.folderUrl
    }

    Connections {
        target: folderModel
        function onStatusChanged() {
            if (control._scrollPending && folderModel.status === FolderListModel.Ready) {
                control._scrollPending = false
                Qt.callLater(_doScrollToLatest)
            }
        }
    }

    function _doScrollToLatest() {
        if (folderModel.count <= 0) return
        var lastIndex = folderModel.count - 1
        _selectedIndex = lastIndex
        _selectedFilePath = folderModel.get(lastIndex, "filePath")
        fileSelected(_selectedFilePath)
        if (_viewMode === "grid") {
            _gridView.positionViewAtIndex(lastIndex, GridView.End)
        } else if (_viewMode === "list") {
            _listView.positionViewAtIndex(lastIndex, ListView.End)
        } else {
            _detailView.positionViewAtIndex(lastIndex, ListView.End)
        }
    }

    function _handleItemClick(index, fp, mouseButton) {
        if (mouseButton === Qt.RightButton) {
            _selectedIndex = index
            _selectedFilePath = fp
            fileSelected(fp)
            _contextMenu.currentFilePath = fp
            _contextMenu.popup()
        } else {
            if (_selectedIndex === index) {
                clearSelection()
            } else {
                _selectedIndex = index
                _selectedFilePath = fp
                fileSelected(fp)
            }
        }
    }

    function _handleItemContextMenu(index, fp, itemRef) {
        _selectedIndex = index
        _selectedFilePath = fp
        fileSelected(fp)
        _contextMenu.currentFilePath = fp
        _contextMenu.popup()
    }

    // ─── FolderListModel ───
    FolderListModel {
        id: folderModel
        folder: control.folderUrl
        nameFilters: control.nameFilters
        showDirs: false
        sortField: control.initialSortField
        sortReversed: !control._sortAscending
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
            sortField: folderModel.sortField
            sortAscending: control._sortAscending
            onViewModeChangeRequested: function(mode) { control._viewMode = mode }
            onSortChangeRequested: function(field, asc) {
                folderModel.sortField = field
                control._sortAscending = asc
                folderModel.sortReversed = !asc
            }
        }

        // Gallery container
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#1E1E1E"
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
            GridView {
                id: _gridView
                anchors.fill: parent
                anchors.margins: 8
                cellWidth: 180
                cellHeight: 160
                clip: true
                visible: control._viewMode === "grid" && folderModel.count > 0
                model: folderModel
                currentIndex: -1
                highlight: Item {}
                highlightFollowsCurrentItem: true
                keyNavigationEnabled: true
                interactive: true
                activeFocusOnTab: true

                onActiveFocusChanged: {
                    if (activeFocus && count > 0 && currentIndex < 0)
                        currentIndex = control._selectedIndex >= 0 ? control._selectedIndex : 0
                    else if (!activeFocus)
                        currentIndex = -1
                }

                Keys.onSpacePressed: {
                    if (currentIndex >= 0 && currentIndex < count) {
                        if (control._selectedIndex === currentIndex) control.clearSelection()
                        else {
                            control._selectedIndex = currentIndex
                            control._selectedFilePath = folderModel.get(currentIndex, "filePath")
                            control.fileSelected(control._selectedFilePath)
                        }
                    }
                }
                Keys.onReturnPressed: _gridView._openMenuAtCurrent()
                Keys.onEnterPressed: _gridView._openMenuAtCurrent()

                function _openMenuAtCurrent() {
                    if (currentIndex < 0 || currentIndex >= count) return
                    control._selectedIndex = currentIndex
                    control._selectedFilePath = folderModel.get(currentIndex, "filePath")
                    control.fileSelected(control._selectedFilePath)
                    _contextMenu.currentFilePath = control._selectedFilePath
                    var item = _gridView.itemAtIndex(currentIndex)
                    if (item) {
                        var pos = item.mapToItem(control, item.width, 0)
                        _contextMenu.popup(control, pos)
                    } else _contextMenu.popup()
                }

                delegate: UTFolderGridDelegate {
                    width: _gridView.cellWidth - (_gridScrollBar.visible ? _gridScrollBar.width / Math.max(1, Math.floor(_gridView.width / _gridView.cellWidth)) : 0)
                    height: _gridView.cellHeight
                    thumbnailComponent: control.gridThumbnail
                    isSelected: control._selectedIndex === index
                    isCurrent: _gridView.currentIndex === index

                    onClicked: function(mouseButton) {
                        _gridView.forceActiveFocus()
                        _gridView.currentIndex = index
                        control._handleItemClick(index, filePath, mouseButton)
                    }
                    onDoubleClicked: control.fileOpenRequested(filePath)
                }

                ScrollBar.vertical: UTScrollBar { id: _gridScrollBar }
            }

            // === List View ===
            ListView {
                id: _listView
                anchors.fill: parent
                anchors.margins: 8
                clip: true
                spacing: 2
                visible: control._viewMode === "list" && folderModel.count > 0
                model: folderModel
                currentIndex: -1
                highlight: Item {}
                highlightFollowsCurrentItem: true
                keyNavigationEnabled: true
                interactive: true
                activeFocusOnTab: true

                onActiveFocusChanged: {
                    if (activeFocus && count > 0 && currentIndex < 0)
                        currentIndex = control._selectedIndex >= 0 ? control._selectedIndex : 0
                    else if (!activeFocus)
                        currentIndex = -1
                }

                Keys.onSpacePressed: {
                    if (currentIndex >= 0 && currentIndex < count) {
                        if (control._selectedIndex === currentIndex) control.clearSelection()
                        else {
                            control._selectedIndex = currentIndex
                            control._selectedFilePath = folderModel.get(currentIndex, "filePath")
                            control.fileSelected(control._selectedFilePath)
                        }
                    }
                }
                Keys.onReturnPressed: _listView._openMenuAtCurrent()
                Keys.onEnterPressed: _listView._openMenuAtCurrent()

                function _openMenuAtCurrent() {
                    if (currentIndex < 0 || currentIndex >= count) return
                    control._selectedIndex = currentIndex
                    control._selectedFilePath = folderModel.get(currentIndex, "filePath")
                    control.fileSelected(control._selectedFilePath)
                    _contextMenu.currentFilePath = control._selectedFilePath
                    var item = _listView.itemAtIndex(currentIndex)
                    if (item) {
                        var pos = item.mapToItem(control, item.width, 0)
                        _contextMenu.popup(control, pos)
                    } else _contextMenu.popup()
                }

                delegate: UTFolderListDelegate {
                    width: _listView.width - 8 - (_listScrollBar.visible ? _listScrollBar.width : 0)
                    thumbnailComponent: control.listThumbnail
                    isSelected: control._selectedIndex === index
                    isCurrent: _listView.currentIndex === index

                    onClicked: function(mouseButton) {
                        _listView.forceActiveFocus()
                        _listView.currentIndex = index
                        control._handleItemClick(index, filePath, mouseButton)
                    }
                    onDoubleClicked: control.fileOpenRequested(filePath)
                }

                ScrollBar.vertical: UTScrollBar { id: _listScrollBar }
            }

            // === Detail View ===
            ListView {
                id: _detailView
                anchors.fill: parent
                anchors.margins: 8
                clip: true
                spacing: 1
                visible: control._viewMode === "detail" && folderModel.count > 0
                model: folderModel
                currentIndex: -1
                highlight: Item {}
                highlightFollowsCurrentItem: true
                keyNavigationEnabled: true
                interactive: true
                activeFocusOnTab: true

                onActiveFocusChanged: {
                    if (activeFocus && count > 0 && currentIndex < 0)
                        currentIndex = control._selectedIndex >= 0 ? control._selectedIndex : 0
                    else if (!activeFocus)
                        currentIndex = -1
                }

                Keys.onSpacePressed: {
                    if (currentIndex >= 0 && currentIndex < count) {
                        if (control._selectedIndex === currentIndex) control.clearSelection()
                        else {
                            control._selectedIndex = currentIndex
                            control._selectedFilePath = folderModel.get(currentIndex, "filePath")
                            control.fileSelected(control._selectedFilePath)
                        }
                    }
                }
                Keys.onReturnPressed: _detailView._openMenuAtCurrent()
                Keys.onEnterPressed: _detailView._openMenuAtCurrent()

                function _openMenuAtCurrent() {
                    if (currentIndex < 0 || currentIndex >= count) return
                    control._selectedIndex = currentIndex
                    control._selectedFilePath = folderModel.get(currentIndex, "filePath")
                    control.fileSelected(control._selectedFilePath)
                    _contextMenu.currentFilePath = control._selectedFilePath
                    var item = _detailView.itemAtIndex(currentIndex)
                    if (item) {
                        var pos = item.mapToItem(control, item.width, 0)
                        _contextMenu.popup(control, pos)
                    } else _contextMenu.popup()
                }

                delegate: UTFolderDetailDelegate {
                    width: _detailView.width - (_detailScrollBar.visible ? _detailScrollBar.width : 0)
                    iconComponent: control.detailIcon
                    isSelected: control._selectedIndex === index
                    isCurrent: _detailView.currentIndex === index

                    onClicked: function(mouseButton) {
                        _detailView.forceActiveFocus()
                        _detailView.currentIndex = index
                        control._handleItemClick(index, filePath, mouseButton)
                    }
                    onDoubleClicked: control.fileOpenRequested(filePath)
                }

                ScrollBar.vertical: UTScrollBar { id: _detailScrollBar }
            }
        }

        // Status bar
        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 4
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
    Menu {
        id: _contextMenu
        property string currentFilePath: ""

        MenuItem {
            text: qsTr("Open")
            onTriggered: control.fileOpenRequested(_contextMenu.currentFilePath)
        }
        MenuItem {
            text: qsTr("Copy to Clipboard")
            onTriggered: control.fileCopyRequested(_contextMenu.currentFilePath)
        }
        MenuItem {
            text: qsTr("Show in Explorer")
            onTriggered: control.fileRevealRequested(_contextMenu.currentFilePath)
        }

        MenuSeparator {
            visible: _extraMenuLoader.item !== null
        }
        Loader {
            id: _extraMenuLoader
            active: control.extraMenuItems !== null
            sourceComponent: control.extraMenuItems
        }

        MenuSeparator {}

        MenuItem {
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
