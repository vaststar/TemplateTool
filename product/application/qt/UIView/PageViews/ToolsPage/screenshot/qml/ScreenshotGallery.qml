import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.folderlistmodel
import UTComponent
import UIResourceLoader 1.0

/**
 * Screenshot Gallery - Browse screenshots folder and capture new screenshots
 *
 * Uses UTFolderView for file browsing with grid/list/detail views.
 */
FocusScope {
    id: root

    required property var controller

    Component.onCompleted: folderView.refresh()
    onVisibleChanged: { if (visible) folderView.refresh() }

    // Screenshot overlay component
    Component {
        id: overlayComponent
        ScreenshotOverlay {
            controller: root.controller
        }
    }

    Connections {
        target: controller

        function onCaptureCompleted(filePath) {
            console.log("Screenshot saved:", filePath)
            folderView.scrollToLatest()
        }

        function onOverlayScreenshotReady(base64, width, height) {
            var overlay = overlayComponent.createObject(null, {
                initialScreenshot: base64,
                imgWidth: width,
                imgHeight: height
            })
            overlay.show()
        }

        function onScreenshotChanged() {
            folderView.refresh()
        }

        function onErrorOccurred(message) {
            console.error("Screenshot error:", message)
        }
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
                focus: true
                onClicked: controller.grabScreenForOverlay()
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
        }

        // === Folder View ===
        UTFolderView {
            id: folderView
            Layout.fillWidth: true
            Layout.fillHeight: true

            folderUrl: controller.screenshotsFolderUrl
            nameFilters: ["*.png", "*.jpg", "*.jpeg", "*.bmp", "*.gif"]
            initialSortField: FolderListModel.Time
            initialSortAscending: true

            emptyIcon: "\ud83d\udcf7"
            emptyTitle: qsTr("No screenshots yet")
            emptyHint: qsTr("Click 'Capture Region' or 'Full Screen' to take a screenshot")
            statusTemplate: qsTr("%1 screenshots")
            deleteDialogTitle: qsTr("Delete Screenshot")
            deleteDialogMessage: qsTr("Are you sure you want to delete this screenshot?")

            onFileOpenRequested: (fp) => controller.openFile(fp)
            onFileCopyRequested: (fp) => controller.copyFileToClipboard(fp)
            onFileRevealRequested: (fp) => controller.revealInFinder(fp)
            onFileDeleteRequested: (fp) => controller.deleteFile(fp)
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
                    fontEnum: UIFontToken.Body_Text_Medium
                    Layout.fillWidth: true
                }

                UTText {
                    text: qsTr("%1 windows").arg(windowPicker.windowModel.length)
                    fontEnum: UIFontToken.Caption_Text
                    colorEnum: UIColorToken.Content_Secondary_Text
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
                visible: windowPicker.windowModel.length > 0
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

                ScrollBar.vertical: UTScrollBar { }
            }

            // Empty state
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: windowPicker.windowModel.length === 0
                spacing: 12

                Item { Layout.fillHeight: true }

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: "🖥"
                    font.pixelSize: 48
                }

                UTText {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("No windows found")
                    fontEnum: UIFontToken.Body_Text
                    colorEnum: UIColorToken.Content_Secondary_Text
                }

                Item { Layout.fillHeight: true }
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
                    colorEnum: UIColorToken.Content_Secondary_Text
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
}
