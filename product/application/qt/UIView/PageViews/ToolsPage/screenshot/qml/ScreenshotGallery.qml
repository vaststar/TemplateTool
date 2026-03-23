import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Window
import Qt.labs.folderlistmodel
import UTComponent
import UIResourceLoader 1.0

/**
 * Screenshot Gallery - Browse screenshots folder and capture new screenshots
 *
 * Features:
 * - Grid (tile) / List view toggle
 * - Click-based selection with highlight border
 * - Capture Region / Full Screen / Window buttons
 */
FocusScope {
    id: root

    required property var controller

    // === State ===
    property int selectedIndex: -1
    property string selectedFilePath: ""
    property bool isGridView: true   // true = grid (tile), false = list

    // Timer-based refresh
    Timer {
        id: refreshTimer
        interval: 100
        repeat: false
        onTriggered: {
            folderModel.folder = controller.screenshotsFolderUrl
        }
    }

    function refreshGallery() {
        folderModel.folder = ""
        refreshTimer.restart()
    }

    function clearSelection() {
        selectedIndex = -1
        selectedFilePath = ""
    }

    Component.onCompleted: {
        refreshGallery()
    }

    onVisibleChanged: {
        if (visible) refreshGallery()
    }

    Connections {
        target: controller
        function onCaptureCompleted(filePath) {
            refreshGallery()
            // Auto-select and scroll to the newest item after model refreshes
            scrollToLatestTimer.restart()
        }
    }

    // Delayed scroll-to-top after model refresh (FolderListModel updates async)
    Timer {
        id: scrollToLatestTimer
        interval: 200
        repeat: false
        onTriggered: {
            if (folderModel.count > 0) {
                selectedIndex = 0
                selectedFilePath = folderModel.get(0, "filePath")
                if (isGridView) {
                    gridView.positionViewAtBeginning()
                } else {
                    listView.positionViewAtBeginning()
                }
            }
        }
    }

    // Screenshot overlay component
    Component {
        id: overlayComponent
        ScreenshotOverlay {
            controller: root.controller
        }
    }

    // Shared folder model
    FolderListModel {
        id: folderModel
        folder: controller.screenshotsFolderUrl
        nameFilters: ["*.png", "*.jpg", "*.jpeg", "*.bmp", "*.gif"]
        showDirs: false
        sortField: FolderListModel.Time
        sortReversed: true
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        // === Top Toolbar ===
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            UTButton {
                text: qsTr("\ud83d\udcf7 Capture Region")
                focus: true  // first tab stop in gallery
                onClicked: {
                    var result = controller.grabScreenForOverlay()
                    if (result.success) {
                        var overlay = overlayComponent.createObject(null, {
                            initialScreenshot: result.base64,
                            imgWidth: result.width,
                            imgHeight: result.height
                        })
                        overlay.show()
                    }
                }
            }

            UTButton {
                text: qsTr("\ud83d\udda5 Full Screen")
                onClicked: controller.captureFullScreen()
            }

            UTButton {
                id: windowCaptureBtn
                text: qsTr("\ud83e\ude9f Window")
                onClicked: windowPicker.open()
            }

            Item { Layout.fillWidth: true }

            // View toggle: grid / list
            RowLayout {
                spacing: 2

                ToolButton {
                    id: gridBtn
                    checked: isGridView
                    onClicked: isGridView = true
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Grid View")

                    UTFocusItem {
                        target: gridBtn
                        focusRadius: 4
                    }

                    contentItem: Text {
                        text: "\u25a6"
                        font.pixelSize: 16
                        color: isGridView ? "#FFFFFF" : "#888888"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        implicitWidth: 32
                        implicitHeight: 32
                        radius: 4
                        color: isGridView ? "#4488FF" : (parent.hovered ? "#3A3A3A" : "transparent")
                    }
                }

                ToolButton {
                    id: listBtn
                    checked: !isGridView
                    onClicked: isGridView = false
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("List View")

                    UTFocusItem {
                        target: listBtn
                        focusRadius: 4
                    }

                    contentItem: Text {
                        text: "\u2630"
                        font.pixelSize: 16
                        color: !isGridView ? "#FFFFFF" : "#888888"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        implicitWidth: 32
                        implicitHeight: 32
                        radius: 4
                        color: !isGridView ? "#4488FF" : (parent.hovered ? "#3A3A3A" : "transparent")
                    }
                }
            }
        }

        // === Gallery Container ===
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
                    text: "\ud83d\udcf7"
                    font.pixelSize: 48
                }

                UTText {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("No screenshots yet")
                    fontEnum: UIFontToken.Body_Text
                    colorEnum: UIColorToken.Content_Secondary
                }

                UTText {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("Click \u0027Capture Region\u0027 or \u0027Full Screen\u0027 to take a screenshot")
                    fontEnum: UIFontToken.Caption_Text
                    colorEnum: UIColorToken.Content_Secondary
                }
            }

            // === Grid View ===
            GridView {
                id: gridView
                anchors.fill: parent
                anchors.margins: 8
                cellWidth: 180
                cellHeight: 160
                clip: true
                visible: isGridView && folderModel.count > 0
                model: folderModel
                currentIndex: -1
                highlight: Item {}
                highlightFollowsCurrentItem: true
                keyNavigationEnabled: true
                interactive: true
                activeFocusOnTab: true

                // Sync currentIndex with selectedIndex when Tab enters
                onActiveFocusChanged: {
                    if (activeFocus && count > 0) {
                        currentIndex = selectedIndex >= 0 ? selectedIndex : 0
                    } else if (!activeFocus) {
                        currentIndex = -1
                    }
                }

                // Space key: select/deselect current item
                Keys.onSpacePressed: {
                    if (currentIndex >= 0 && currentIndex < count) {
                        if (selectedIndex === currentIndex) {
                            clearSelection()
                        } else {
                            selectedIndex = currentIndex
                            selectedFilePath = folderModel.get(currentIndex, "filePath")
                        }
                    }
                }

                // Enter/Return key: show context menu at the current item
                Keys.onReturnPressed: { gridView.openMenuAtCurrentItem() }
                Keys.onEnterPressed:  { gridView.openMenuAtCurrentItem() }

                function openMenuAtCurrentItem() {
                    if (currentIndex < 0 || currentIndex >= count) return
                    selectedIndex = currentIndex
                    selectedFilePath = folderModel.get(currentIndex, "filePath")
                    contextMenu.currentFilePath = selectedFilePath
                    // Position menu next to the current delegate item
                    var item = gridView.itemAtIndex(currentIndex)
                    if (item) {
                        var pos = item.mapToItem(root, item.width, 0)
                        contextMenu.popup(root, pos)
                    } else {
                        contextMenu.popup()
                    }
                }

                delegate: Item {
                    width: gridView.cellWidth - (gridScrollBar.visible ? gridScrollBar.width / Math.floor(gridView.width / gridView.cellWidth) : 0)
                    height: gridView.cellHeight

                    Rectangle {
                        id: gridCard
                        anchors.fill: parent
                        anchors.margins: 4
                        radius: 8
                        color: isSelected ? "#2A3A5A" : (gridMouseArea.containsMouse ? "#333333" : "#2A2A2A")
                        border.width: 0

                        property bool isSelected: selectedIndex === index

                        UTFocusItem {
                            delegateFocused: gridCard.parent.GridView.isCurrentItem
                            focusRadius: gridCard.radius
                        }

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 4

                            Image {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                source: fileUrl
                                fillMode: Image.PreserveAspectFit
                                asynchronous: true
                                cache: true
                                smooth: true
                                mipmap: true

                                BusyIndicator {
                                    anchors.centerIn: parent
                                    running: parent.status === Image.Loading
                                    visible: running
                                }
                            }

                            UTText {
                                Layout.fillWidth: true
                                text: fileName
                                font.pixelSize: 11
                                color: gridCard.isSelected ? "#FFFFFF" : "#CCCCCC"
                                elide: Text.ElideMiddle
                                horizontalAlignment: Text.AlignHCenter
                            }

                            UTText {
                                Layout.fillWidth: true
                                text: Qt.formatDateTime(fileModified, "yyyy-MM-dd hh:mm")
                                font.pixelSize: 10
                                color: gridCard.isSelected ? "#BBBBBB" : "#888888"
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }

                        MouseArea {
                            id: gridMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            acceptedButtons: Qt.LeftButton | Qt.RightButton

                            onClicked: function(mouse) {
                                gridView.forceActiveFocus()
                                gridView.currentIndex = index
                                if (mouse.button === Qt.RightButton) {
                                    selectedIndex = index
                                    selectedFilePath = filePath
                                    contextMenu.currentFilePath = filePath
                                    contextMenu.popup()
                                } else {
                                    if (selectedIndex === index) {
                                        clearSelection()
                                    } else {
                                        selectedIndex = index
                                        selectedFilePath = filePath
                                    }
                                }
                            }

                            onDoubleClicked: controller.openFile(filePath)
                        }
                    }
                }

                ScrollBar.vertical: ScrollBar {
                    id: gridScrollBar
                }
            }

            // === List View ===
            ListView {
                id: listView
                anchors.fill: parent
                anchors.margins: 8
                clip: true
                spacing: 2
                visible: !isGridView && folderModel.count > 0
                model: folderModel
                currentIndex: -1
                highlight: Item {}
                highlightFollowsCurrentItem: true
                keyNavigationEnabled: true
                interactive: true
                activeFocusOnTab: true

                // Sync currentIndex with selectedIndex when Tab enters
                onActiveFocusChanged: {
                    if (activeFocus && count > 0) {
                        currentIndex = selectedIndex >= 0 ? selectedIndex : 0
                    } else if (!activeFocus) {
                        currentIndex = -1
                    }
                }

                // Space key: select/deselect current item
                Keys.onSpacePressed: {
                    if (currentIndex >= 0 && currentIndex < count) {
                        if (selectedIndex === currentIndex) {
                            clearSelection()
                        } else {
                            selectedIndex = currentIndex
                            selectedFilePath = folderModel.get(currentIndex, "filePath")
                        }
                    }
                }

                // Enter/Return key: show context menu at the current item
                Keys.onReturnPressed: { listView.openMenuAtCurrentItem() }
                Keys.onEnterPressed:  { listView.openMenuAtCurrentItem() }

                function openMenuAtCurrentItem() {
                    if (currentIndex < 0 || currentIndex >= count) return
                    selectedIndex = currentIndex
                    selectedFilePath = folderModel.get(currentIndex, "filePath")
                    contextMenu.currentFilePath = selectedFilePath
                    // Position menu next to the current delegate item
                    var item = listView.itemAtIndex(currentIndex)
                    if (item) {
                        var pos = item.mapToItem(root, item.width, 0)
                        contextMenu.popup(root, pos)
                    } else {
                        contextMenu.popup()
                    }
                }

                delegate: Rectangle {
                    id: listCard
                    width: listView.width - 8 - (listScrollBar.visible ? listScrollBar.width : 0)
                    anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined
                    height: 56
                    radius: 6
                    color: isSelected ? "#2A3A5A" : (listMouseArea.containsMouse ? "#333333" : "transparent")
                    border.width: 0

                    property bool isSelected: selectedIndex === index

                    UTFocusItem {
                        delegateFocused: listCard.ListView.isCurrentItem
                        focusRadius: listCard.radius
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 12
                        spacing: 12

                        // Thumbnail
                        Rectangle {
                            width: 44
                            height: 44
                            radius: 4
                            color: "#2A2A2A"

                            Image {
                                anchors.fill: parent
                                anchors.margins: 2
                                source: fileUrl
                                fillMode: Image.PreserveAspectFit
                                asynchronous: true
                                cache: true
                                smooth: true
                                mipmap: true

                                BusyIndicator {
                                    anchors.centerIn: parent
                                    running: parent.status === Image.Loading
                                    visible: running
                                    width: 20
                                    height: 20
                                }
                            }
                        }

                        // File info
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2

                            UTText {
                                text: fileName
                                font.pixelSize: 13
                                color: listCard.isSelected ? "#FFFFFF" : "#DDDDDD"
                                elide: Text.ElideMiddle
                                Layout.fillWidth: true
                            }

                            UTText {
                                text: Qt.formatDateTime(fileModified, "yyyy-MM-dd hh:mm:ss")
                                font.pixelSize: 11
                                color: listCard.isSelected ? "#BBBBBB" : "#888888"
                            }
                        }

                        // File size
                        UTText {
                            text: {
                                var s = fileSize
                                if (s === undefined || s === 0) return ""
                                if (s > 1048576) return (s / 1048576).toFixed(1) + " MB"
                                return (s / 1024).toFixed(0) + " KB"
                            }
                            font.pixelSize: 11
                            color: "#888888"
                            Layout.preferredWidth: 60
                            horizontalAlignment: Text.AlignRight
                        }
                    }

                    MouseArea {
                        id: listMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.LeftButton | Qt.RightButton

                        onClicked: function(mouse) {
                            listView.forceActiveFocus()
                            listView.currentIndex = index
                            if (mouse.button === Qt.RightButton) {
                                selectedIndex = index
                                selectedFilePath = filePath
                                contextMenu.currentFilePath = filePath
                                contextMenu.popup()
                            } else {
                                if (selectedIndex === index) {
                                    clearSelection()
                                } else {
                                    selectedIndex = index
                                    selectedFilePath = filePath
                                }
                            }
                        }

                        onDoubleClicked: controller.openFile(filePath)
                    }
                }

                ScrollBar.vertical: ScrollBar {
                    id: listScrollBar
                }
            }
        }

        // === Status Bar ===
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            UTText {
                text: qsTr("%1 screenshots").arg(folderModel.count)
                fontEnum: UIFontToken.Caption_Text
                colorEnum: UIColorToken.Content_Secondary
            }

            Item { Layout.fillWidth: true }
        }
    }

    // === Context Menu ===
    Menu {
        id: contextMenu
        property string currentFilePath: ""

        MenuItem {
            text: qsTr("Open")
            onTriggered: controller.openFile(contextMenu.currentFilePath)
        }

        MenuItem {
            text: qsTr("Copy to Clipboard")
            onTriggered: controller.copyFileToClipboard(contextMenu.currentFilePath)
        }

        MenuItem {
            text: qsTr("Show in Finder")
            onTriggered: controller.revealInFinder(contextMenu.currentFilePath)
        }

        MenuSeparator {}

        MenuItem {
            text: qsTr("Delete")
            onTriggered: {
                deleteConfirmDialog.targetFilePath = contextMenu.currentFilePath
                deleteConfirmDialog.open()
            }
        }
    }

    // === Delete Confirmation Dialog ===
    Dialog {
        id: deleteConfirmDialog
        property string targetFilePath: ""
        title: qsTr("Delete Screenshot")
        modal: true
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: Math.min(400, root.width * 0.8)
        standardButtons: Dialog.Yes | Dialog.No

        contentItem: UTText {
            text: qsTr("Are you sure you want to delete this screenshot?")
            fontEnum: UIFontToken.Body_Text
        }

        onAccepted: {
            controller.deleteFile(targetFilePath)
            clearSelection()
            refreshGallery()
        }
    }

    // === Window Picker Popup ===
    Popup {
        id: windowPicker
        anchors.centerIn: parent
        width: Math.min(540, root.width * 0.85)
        height: Math.min(520, root.height * 0.85)
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        padding: 0

        property var windowModel: []
        // Cache of loaded thumbnail base64 strings keyed by windowId
        property var thumbnailCache: ({})

        onOpened: {
            windowModel = controller.getWindowList()
            thumbnailCache = {}
            // Focus the grid and select first item
            windowGridView.forceActiveFocus()
            windowGridView.currentIndex = windowModel.length > 0 ? 0 : -1
        }
        onClosed: {
            windowModel = []
            thumbnailCache = {}
            // Return focus to the window capture button
            windowCaptureBtn.forceActiveFocus()
        }

        background: Rectangle {
            radius: 12
            color: "#1E1E1E"
            border.color: "#3A3A3A"
            border.width: 1
        }

        contentItem: ColumnLayout {
            spacing: 0

            // Header
            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                Layout.topMargin: 16
                Layout.bottomMargin: 12
                spacing: 12

                UTText {
                    text: qsTr("Select a Window to Capture")
                    fontEnum: UIFontToken.Body_Text_Bold
                    Layout.fillWidth: true
                }

                UTText {
                    text: qsTr("%1 windows").arg(windowPicker.windowModel.length)
                    fontEnum: UIFontToken.Caption_Text
                    colorEnum: UIColorToken.Content_Secondary
                }
            }

            // Separator
            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: "#333333"
            }

            // Grid of window cards
            GridView {
                id: windowGridView
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.margins: 12
                cellWidth: 200
                cellHeight: 170
                clip: true
                model: windowPicker.windowModel
                currentIndex: -1
                highlight: Item {}
                highlightFollowsCurrentItem: true
                keyNavigationEnabled: true
                activeFocusOnTab: false  // managed manually: popup open → grid, Tab → cancel btn

                onActiveFocusChanged: {
                    if (activeFocus && count > 0) {
                        if (currentIndex < 0) currentIndex = 0
                    } else if (!activeFocus) {
                        currentIndex = -1
                    }
                }

                // Tab from grid → Cancel button
                Keys.onTabPressed: {
                    cancelBtn.forceActiveFocus()
                }

                Keys.onReturnPressed: {
                    if (currentIndex >= 0 && currentIndex < count) {
                        var item = windowPicker.windowModel[currentIndex]
                        if (item) {
                            controller.captureWindow(item.windowId)
                            windowPicker.close()
                        }
                    }
                }
                Keys.onEnterPressed: Keys.onReturnPressed(event)

                delegate: Item {
                    width: windowGridView.cellWidth
                    height: windowGridView.cellHeight

                    Rectangle {
                        id: windowCard
                        anchors.fill: parent
                        anchors.margins: 6
                        radius: 10
                        color: windowCardMouse.containsMouse ? "#333333" : "#262626"
                        border.width: 1
                        border.color: windowCardMouse.containsMouse ? "#555555" : "#3A3A3A"

                        UTFocusItem {
                            delegateFocused: windowCard.parent.GridView.isCurrentItem
                            focusRadius: windowCard.radius
                        }

                        // Subtle scale on hover
                        scale: windowCardMouse.containsMouse ? 1.02 : 1.0
                        Behavior on scale { NumberAnimation { duration: 120; easing.type: Easing.OutCubic } }
                        Behavior on color { ColorAnimation { duration: 120 } }
                        Behavior on border.color { ColorAnimation { duration: 120 } }

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 6

                            // Thumbnail area
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                radius: 6
                                color: "#1A1A1A"
                                clip: true

                                // Thumbnail image (lazy-loaded)
                                Image {
                                    id: thumbImg
                                    anchors.fill: parent
                                    anchors.margins: 2
                                    fillMode: Image.PreserveAspectFit
                                    asynchronous: false
                                    cache: true
                                    smooth: true
                                    mipmap: true
                                    visible: source.toString().length > 0

                                    property string windowIdKey: String(modelData.windowId)

                                    // Load thumbnail when delegate becomes visible
                                    Component.onCompleted: {
                                        loadThumbnail()
                                    }

                                    function loadThumbnail() {
                                        var cached = windowPicker.thumbnailCache[windowIdKey]
                                        if (cached) {
                                            source = "data:image/png;base64," + cached
                                            return
                                        }
                                        // Load asynchronously via a timer so grid renders quickly first
                                        thumbLoadTimer.restart()
                                    }

                                    Timer {
                                        id: thumbLoadTimer
                                        interval: 50 + index * 80  // stagger loads
                                        repeat: false
                                        onTriggered: {
                                            var b64 = controller.getWindowThumbnailBase64(modelData.windowId)
                                            if (b64.length > 0) {
                                                var newCache = windowPicker.thumbnailCache
                                                newCache[thumbImg.windowIdKey] = b64
                                                windowPicker.thumbnailCache = newCache
                                                thumbImg.source = "data:image/png;base64," + b64
                                            }
                                        }
                                    }
                                }

                                // Placeholder while loading
                                ColumnLayout {
                                    anchors.centerIn: parent
                                    visible: thumbImg.source.toString().length === 0
                                    spacing: 4

                                    Text {
                                        text: "🖥"
                                        font.pixelSize: 28
                                        color: "#555555"
                                        Layout.alignment: Qt.AlignHCenter
                                    }
                                    BusyIndicator {
                                        running: true
                                        Layout.alignment: Qt.AlignHCenter
                                        width: 20
                                        height: 20
                                    }
                                }

                                // Hover overlay with "Capture" hint
                                Rectangle {
                                    anchors.fill: parent
                                    radius: 6
                                    color: "#44000000"
                                    visible: windowCardMouse.containsMouse && thumbImg.source.toString().length > 0

                                    Text {
                                        anchors.centerIn: parent
                                        text: "📷 " + qsTr("Click to Capture")
                                        font.pixelSize: 13
                                        font.bold: true
                                        color: "#FFFFFF"
                                    }
                                }
                            }

                            // Window name
                            Text {
                                Layout.fillWidth: true
                                text: modelData.name || qsTr("Unnamed Window")
                                font.pixelSize: 12
                                font.bold: true
                                color: windowCardMouse.containsMouse ? "#FFFFFF" : "#DDDDDD"
                                elide: Text.ElideRight
                                maximumLineCount: 1
                                Behavior on color { ColorAnimation { duration: 120 } }
                            }

                            // Owner name
                            Text {
                                Layout.fillWidth: true
                                text: modelData.ownerName || ""
                                font.pixelSize: 11
                                color: "#888888"
                                elide: Text.ElideRight
                                maximumLineCount: 1
                                visible: text.length > 0
                            }
                        }

                        // Click / press visual feedback
                        Rectangle {
                            anchors.fill: parent
                            radius: 10
                            color: windowCardMouse.pressed ? "#224488FF" : "transparent"
                        }

                        MouseArea {
                            id: windowCardMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor

                            onClicked: {
                                controller.captureWindow(modelData.windowId)
                                windowPicker.close()
                            }
                        }
                    }
                }

                ScrollBar.vertical: ScrollBar { }

                // Empty state
                ColumnLayout {
                    anchors.centerIn: parent
                    visible: windowPicker.windowModel.length === 0
                    spacing: 12

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: "🖥"
                        font.pixelSize: 48
                    }

                    UTText {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("No windows found")
                        fontEnum: UIFontToken.Body_Text
                        colorEnum: UIColorToken.Content_Secondary
                    }
                }
            }

            // Separator
            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: "#333333"
            }

            // Footer
            RowLayout {
                Layout.fillWidth: true
                Layout.margins: 16
                spacing: 12

                UTText {
                    text: qsTr("Hover to preview, click to capture")
                    fontEnum: UIFontToken.Caption_Text
                    colorEnum: UIColorToken.Content_Secondary
                    Layout.fillWidth: true
                }

                UTButton {
                    id: cancelBtn
                    text: qsTr("Cancel")
                    onClicked: windowPicker.close()
                    // Shift+Tab goes back to grid
                    Keys.onBacktabPressed: {
                        windowGridView.forceActiveFocus()
                    }
                    // Enter on Cancel closes popup
                    Keys.onReturnPressed: windowPicker.close()
                    Keys.onEnterPressed: windowPicker.close()
                }
            }
        }
    }

    // === Signal Connections ===
    Connections {
        target: controller

        function onScreenshotChanged() {
            refreshGallery()
        }

        function onCaptureCompleted(filePath) {
            console.log("Screenshot saved:", filePath)
            refreshGallery()
        }

        function onErrorOccurred(message) {
            console.error("Screenshot error:", message)
        }
    }
}
