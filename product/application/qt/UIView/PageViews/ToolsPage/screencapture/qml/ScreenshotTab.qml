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
Item {
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
                currentIndex: selectedIndex

                delegate: Item {
                    width: gridView.cellWidth
                    height: gridView.cellHeight

                    Rectangle {
                        id: gridCard
                        anchors.fill: parent
                        anchors.margins: 4
                        radius: 8
                        color: isSelected ? "#2A3A5A" : (gridMouseArea.containsMouse ? "#333333" : "#2A2A2A")
                        border.width: isSelected ? 2 : 0
                        border.color: "#4488FF"

                        property bool isSelected: selectedIndex === index

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

                ScrollBar.vertical: ScrollBar { }
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
                currentIndex: selectedIndex

                delegate: Rectangle {
                    id: listCard
                    width: listView.width
                    height: 56
                    radius: 6
                    color: isSelected ? "#2A3A5A" : (listMouseArea.containsMouse ? "#333333" : "transparent")
                    border.width: isSelected ? 2 : 0
                    border.color: "#4488FF"

                    property bool isSelected: selectedIndex === index

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

                ScrollBar.vertical: ScrollBar { }
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
        width: 400
        height: 500
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        onOpened: windowListView.model = controller.getWindowList()
        onClosed: windowListView.model = []

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            UTText {
                text: qsTr("Select Window")
                fontEnum: UIFontToken.Body_Text_Bold
            }

            ListView {
                id: windowListView
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: []

                delegate: ItemDelegate {
                    width: ListView.view.width
                    height: 48

                    contentItem: ColumnLayout {
                        spacing: 2

                        UTText {
                            text: modelData.name || qsTr("Unnamed Window")
                            fontEnum: UIFontToken.Body_Text
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }

                        UTText {
                            text: modelData.ownerName || ""
                            fontEnum: UIFontToken.Caption_Text
                            colorEnum: UIColorToken.Content_Secondary
                            visible: modelData.ownerName && modelData.ownerName.length > 0
                        }
                    }

                    onClicked: {
                        controller.captureWindow(modelData.windowId)
                        windowPicker.close()
                    }
                }

                ScrollBar.vertical: ScrollBar { }
            }

            UTButton {
                Layout.alignment: Qt.AlignRight
                text: qsTr("Cancel")
                onClicked: windowPicker.close()
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
