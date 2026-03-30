import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.folderlistmodel
import UTComponent
import UIResourceLoader 1.0

/**
 * Recording Content - Gallery of recorded videos with capture controls
 *
 * Layout:
 *   Top toolbar  : Start Recording, Mode selector, Open Folder, View toggle
 *   Gallery      : Grid / List of video files from the recordings directory
 *   Status bar   : File count + selected file info + actions
 */
FocusScope {
    id: root

    required property var controller

    // === State ===
    property int selectedIndex: -1
    property string selectedFilePath: ""
    property bool isGridView: true

    // Timer-based refresh
    Timer {
        id: refreshTimer
        interval: 100
        repeat: false
        onTriggered: {
            folderModel.folder = "file://" + controller.getDefaultSavePath()
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

    function formatDuration(seconds) {
        var h = Math.floor(seconds / 3600)
        var m = Math.floor((seconds % 3600) / 60)
        var s = seconds % 60
        if (h > 0) {
            return String(h).padStart(2, '0') + ":" +
                   String(m).padStart(2, '0') + ":" +
                   String(s).padStart(2, '0')
        }
        return String(m).padStart(2, '0') + ":" + String(s).padStart(2, '0')
    }

    Component.onCompleted: refreshGallery()
    onVisibleChanged: { if (visible) refreshGallery() }

    Connections {
        target: controller

        function onRecordingCompleted(filePath) {
            refreshGallery()
            scrollToLatestTimer.restart()
        }

        function onErrorOccurred(message) {
            errorText.text = message
            errorText.visible = true
            errorHideTimer.restart()
            // Cancel pending floating bar on error
            pendingFullscreenBar = false
        }

        // Clean up floating bar when recording stops; show bar when recording starts
        function onRecordingStateChanged() {
            if (controller.isRecording && pendingFullscreenBar && !activeFloatingBar) {
                // Recording started successfully — now show the floating bar
                activeFloatingBar = floatingBarComponent.createObject(null)
                activeFloatingBar.show()
                pendingFullscreenBar = false
            }
            if (!controller.isRecording) {
                pendingFullscreenBar = false
                if (activeFloatingBar) {
                    activeFloatingBar.close()
                    activeFloatingBar.destroy()
                    activeFloatingBar = null
                }
            }
        }
    }

    Timer {
        id: scrollToLatestTimer
        interval: 300
        repeat: false
        onTriggered: {
            if (folderModel.count > 0) {
                selectedIndex = 0
                selectedFilePath = folderModel.get(0, "filePath")
                if (isGridView) gridView.positionViewAtBeginning()
                else listView.positionViewAtBeginning()
            }
        }
    }

    Timer {
        id: errorHideTimer
        interval: 5000
        onTriggered: errorText.visible = false
    }

    // Shared folder model
    FolderListModel {
        id: folderModel
        folder: ""
        nameFilters: ["*.mp4", "*.webm", "*.mov", "*.mkv", "*.avi"]
        showDirs: false
        sortField: FolderListModel.Time
        sortReversed: true
    }

    // Region selector component
    Component {
        id: regionSelectorComponent
        RecordingRegionSelector {
            controller: root.controller
            onRecordingFinished: function(filePath) {
                root.activeRegionSelector = null
                root.refreshGallery()
                scrollToLatestTimer.restart()
            }
            Component.onDestruction: {
                if (root.activeRegionSelector === this)
                    root.activeRegionSelector = null
            }
        }
    }

    // Floating bar component (for fullscreen recording)
    Component {
        id: floatingBarComponent
        RecordingFloatingBar {
            controller: root.controller
        }
    }

    // Keep track of active floating bar for fullscreen mode
    property var activeFloatingBar: null
    // Keep track of active region selector to prevent GC
    property var activeRegionSelector: null
    // Track if we initiated a fullscreen recording (to know when to show bar)
    property bool pendingFullscreenBar: false

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        // === Top Toolbar ===
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            UTButton {
                id: startBtn
                text: qsTr("🎬 Start Recording")
                focus: true
                enabled: controller.ffmpegAvailable && !controller.isRecording

                onClicked: {
                    if (modeCombo.currentValue === "region") {
                        activeRegionSelector = regionSelectorComponent.createObject(null)
                        if (Qt.platform.os === "osx") {
                            // macOS: show() instead of showFullScreen() to avoid
                            // entering a native Space. The window already covers
                            // the screen via manual sizing in Component.onCompleted.
                            activeRegionSelector.show()
                        } else {
                            activeRegionSelector.showFullScreen()
                        }
                    } else {
                        pendingFullscreenBar = true
                        controller.startRecording("fullscreen")
                    }
                }
            }

            ComboBox {
                id: modeCombo
                model: [
                    { text: qsTr("Full Screen"), value: "fullscreen" },
                    { text: qsTr("Select Region"), value: "region" }
                ]
                textRole: "text"
                valueRole: "value"
                currentIndex: 0
                enabled: !controller.isRecording
                implicitWidth: 140
            }

            // FFmpeg warning (only when unavailable)
            UTText {
                visible: !controller.ffmpegAvailable
                text: qsTr("⚠ FFmpeg not found")
                fontEnum: UIFontToken.Caption_Text
                color: "#FF6B6B"
            }

            Item { Layout.fillWidth: true }

            UTButton {
                text: qsTr("📂 Open Folder")
                onClicked: controller.openRecordingsFolder()
            }

            // View toggle: grid / list
            RowLayout {
                spacing: 2

                ToolButton {
                    id: gridBtn
                    checked: isGridView
                    onClicked: isGridView = true
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Grid View")

                    contentItem: Text {
                        text: "\u25A6"
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

        // === Error Banner ===
        UTText {
            id: errorText
            visible: false
            Layout.fillWidth: true
            color: "#FF6B6B"
            fontEnum: UIFontToken.Caption_Text
            wrapMode: Text.WordWrap
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
                    text: "🎬"
                    font.pixelSize: 48
                }

                UTText {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("No recordings yet")
                    fontEnum: UIFontToken.Body_Text
                    colorEnum: UIColorToken.Content_Secondary_Text
                }

                UTText {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("Click 'Start Recording' to capture your screen")
                    fontEnum: UIFontToken.Caption_Text
                    colorEnum: UIColorToken.Content_Secondary_Text
                }
            }

            // === Grid View ===
            GridView {
                id: gridView
                anchors.fill: parent
                anchors.margins: 8
                cellWidth: 200
                cellHeight: 140
                clip: true
                visible: isGridView && folderModel.count > 0
                model: folderModel
                currentIndex: -1
                highlight: Item {}
                highlightFollowsCurrentItem: true
                keyNavigationEnabled: true
                interactive: true
                activeFocusOnTab: true

                onActiveFocusChanged: {
                    if (activeFocus && count > 0)
                        currentIndex = selectedIndex >= 0 ? selectedIndex : 0
                    else if (!activeFocus)
                        currentIndex = -1
                }

                Keys.onSpacePressed: {
                    if (currentIndex >= 0 && currentIndex < count) {
                        if (selectedIndex === currentIndex) clearSelection()
                        else {
                            selectedIndex = currentIndex
                            selectedFilePath = folderModel.get(currentIndex, "filePath")
                        }
                    }
                }

                Keys.onReturnPressed: gridView.openMenuAtCurrentItem()
                Keys.onEnterPressed:  gridView.openMenuAtCurrentItem()

                function openMenuAtCurrentItem() {
                    if (currentIndex < 0 || currentIndex >= count) return
                    selectedIndex = currentIndex
                    selectedFilePath = folderModel.get(currentIndex, "filePath")
                    contextMenu.currentFilePath = selectedFilePath
                    var item = gridView.itemAtIndex(currentIndex)
                    if (item) {
                        var pos = item.mapToItem(root, item.width, 0)
                        contextMenu.popup(root, pos)
                    } else contextMenu.popup()
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

                        property bool isSelected: selectedIndex === index

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 4

                            // Video icon placeholder
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                color: "#222222"
                                radius: 4

                                Text {
                                    anchors.centerIn: parent
                                    text: "🎥"
                                    font.pixelSize: 32
                                    opacity: 0.6
                                }

                                // Format badge
                                Rectangle {
                                    anchors.right: parent.right
                                    anchors.bottom: parent.bottom
                                    anchors.margins: 4
                                    width: fmtLabel.width + 8
                                    height: fmtLabel.height + 4
                                    radius: 3
                                    color: "#44000000"

                                    Text {
                                        id: fmtLabel
                                        anchors.centerIn: parent
                                        text: {
                                            var ext = fileName.split('.').pop()
                                            return ext ? ext.toUpperCase() : ""
                                        }
                                        font.pixelSize: 9
                                        font.bold: true
                                        color: "#CCCCCC"
                                    }
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
                                    if (selectedIndex === index) clearSelection()
                                    else {
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

                onActiveFocusChanged: {
                    if (activeFocus && count > 0)
                        currentIndex = selectedIndex >= 0 ? selectedIndex : 0
                    else if (!activeFocus)
                        currentIndex = -1
                }

                Keys.onSpacePressed: {
                    if (currentIndex >= 0 && currentIndex < count) {
                        if (selectedIndex === currentIndex) clearSelection()
                        else {
                            selectedIndex = currentIndex
                            selectedFilePath = folderModel.get(currentIndex, "filePath")
                        }
                    }
                }

                Keys.onReturnPressed: listView.openMenuAtCurrentItem()
                Keys.onEnterPressed:  listView.openMenuAtCurrentItem()

                function openMenuAtCurrentItem() {
                    if (currentIndex < 0 || currentIndex >= count) return
                    selectedIndex = currentIndex
                    selectedFilePath = folderModel.get(currentIndex, "filePath")
                    contextMenu.currentFilePath = selectedFilePath
                    var item = listView.itemAtIndex(currentIndex)
                    if (item) {
                        var pos = item.mapToItem(root, item.width, 0)
                        contextMenu.popup(root, pos)
                    } else contextMenu.popup()
                }

                delegate: Rectangle {
                    id: listCard
                    width: listView.width - 8 - (listScrollBar.visible ? listScrollBar.width : 0)
                    anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined
                    height: 56
                    radius: 6
                    color: isSelected ? "#2A3A5A" : (listMouseArea.containsMouse ? "#333333" : "transparent")

                    property bool isSelected: selectedIndex === index

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 12
                        spacing: 12

                        // Video icon
                        Rectangle {
                            width: 44
                            height: 44
                            radius: 4
                            color: "#2A2A2A"

                            Text {
                                anchors.centerIn: parent
                                text: "🎥"
                                font.pixelSize: 20
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
                                if (selectedIndex === index) clearSelection()
                                else {
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
                text: qsTr("%1 recordings").arg(folderModel.count)
                fontEnum: UIFontToken.Caption_Text
                colorEnum: UIColorToken.Content_Secondary_Text
            }

            Item { Layout.fillWidth: true }

            // Selected file actions
            UTButton {
                visible: selectedFilePath.length > 0
                text: qsTr("▶ Open")
                onClicked: controller.openFile(selectedFilePath)
            }

            UTButton {
                visible: selectedFilePath.length > 0
                text: qsTr("🗑 Delete")
                onClicked: {
                    deleteConfirmDialog.targetFilePath = selectedFilePath
                    deleteConfirmDialog.open()
                }
            }
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
            text: qsTr("Show in Explorer")
            onTriggered: controller.openRecordingsFolder()
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
        title: qsTr("Delete Recording")
        modal: true
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: Math.min(400, root.width * 0.8)
        standardButtons: Dialog.Yes | Dialog.No

        contentItem: UTText {
            text: qsTr("Are you sure you want to delete this recording?")
            fontEnum: UIFontToken.Body_Text
        }

        onAccepted: {
            controller.deleteFile(targetFilePath)
            clearSelection()
            refreshGallery()
        }
    }
}
