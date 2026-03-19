import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Window
import Qt.labs.folderlistmodel
import UTComponent
import UIResourceLoader 1.0

/**
 * Screenshot Tab - Browse screenshots folder and capture new screenshots
 *
 * Design:
 * - Main view: Grid of existing screenshots in the screenshots folder
 * - Capture: Click → fullscreen overlay → select region → edit → save
 */
Item {
    id: root

    required property var controller

    // Screenshot overlay component
    Component {
        id: overlayComponent
        ScreenshotOverlay {
            controller: root.controller
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 12

        // === Top Toolbar ===
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            UTButton {
                text: qsTr("📷 Capture Region")
                onClicked: {
                    // First capture the screen, then show overlay
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
                text: qsTr("🖥 Full Screen")
                onClicked: controller.captureFullScreen()
            }

            UTButton {
                text: qsTr("🪟 Window")
                onClicked: windowPicker.open()
            }

            Item { Layout.fillWidth: true }

            UTButton {
                text: qsTr("📂 Open Folder")
                onClicked: controller.openScreenshotsFolder()
            }

            UTButton {
                text: qsTr("🔄")
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Refresh")
                onClicked: {
                    folderModel.folder = ""
                    folderModel.folder = controller.screenshotsFolderUrl
                }
            }
        }

        // === Folder Path Display ===
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            UTText {
                text: qsTr("Folder:")
                fontEnum: UIFontToken.Caption_Text
                colorEnum: UIColorToken.Content_Secondary
            }

            UTText {
                text: controller.outputDirectory
                fontEnum: UIFontToken.Caption_Text
                colorEnum: UIColorToken.Content_Secondary
                elide: Text.ElideMiddle
                Layout.fillWidth: true
            }

            UTButton {
                text: qsTr("Change...")
                implicitHeight: 28
                onClicked: folderDialog.open()
            }
        }

        // === Screenshots Grid ===
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#1E1E1E"  // Dark background
            radius: 8

            // Empty state
            ColumnLayout {
                anchors.centerIn: parent
                visible: gridView.count === 0
                spacing: 16

                UTText {
                    Layout.alignment: Qt.AlignHCenter
                    text: "📷"
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
                    text: qsTr("Click 'Capture Region' or 'Full Screen' to take a screenshot")
                    fontEnum: UIFontToken.Caption_Text
                    colorEnum: UIColorToken.Content_Secondary
                }
            }

            // Screenshots grid
            GridView {
                id: gridView
                anchors.fill: parent
                anchors.margins: 12
                cellWidth: 160
                cellHeight: 140
                clip: true

                model: FolderListModel {
                    id: folderModel
                    folder: controller.screenshotsFolderUrl
                    nameFilters: ["*.png", "*.jpg", "*.jpeg", "*.bmp", "*.gif"]
                    showDirs: false
                    sortField: FolderListModel.Time
                    sortReversed: true  // Newest first
                }

                delegate: Rectangle {
                    width: gridView.cellWidth - 8
                    height: gridView.cellHeight - 8
                    color: delegateMouseArea.containsMouse ?
                           UTComponentUtil.getPlainUIColor(UIColorToken.Surface_Hover, UIColorState.Normal) :
                           "#2A2A2A"  // Dark background for better text visibility
                    radius: 8

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 4

                        // Thumbnail
                        Image {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            source: fileUrl
                            fillMode: Image.PreserveAspectFit
                            asynchronous: true
                            cache: true

                            BusyIndicator {
                                anchors.centerIn: parent
                                running: parent.status === Image.Loading
                                visible: running
                            }
                        }

                        // Filename
                        UTText {
                            Layout.fillWidth: true
                            text: fileName
                            font.pixelSize: 11
                            color: "#FFFFFF"  // White text for visibility
                            elide: Text.ElideMiddle
                            horizontalAlignment: Text.AlignHCenter
                        }

                        // Date
                        UTText {
                            Layout.fillWidth: true
                            text: Qt.formatDateTime(fileModified, "yyyy-MM-dd hh:mm")
                            font.pixelSize: 10
                            color: "#AAAAAA"  // Gray text
                            horizontalAlignment: Text.AlignHCenter
                        }
                    }

                    MouseArea {
                        id: delegateMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.LeftButton | Qt.RightButton

                        onClicked: function(mouse) {
                            if (mouse.button === Qt.RightButton) {
                                contextMenu.currentFilePath = filePath
                                contextMenu.popup()
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
                text: qsTr("%1 screenshots").arg(gridView.count)
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
            folderModel.folder = ""
            folderModel.folder = controller.screenshotsFolderUrl
        }
    }

    // === Folder Selection Dialog ===
    FolderDialog {
        id: folderDialog
        title: qsTr("Select Screenshots Folder")
        currentFolder: controller.screenshotsFolderUrl
        onAccepted: {
            controller.setOutputDirectory(selectedFolder.toString().replace("file://", ""))
            folderModel.folder = controller.screenshotsFolderUrl
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

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            UTText {
                text: qsTr("Select Window")
                fontEnum: UIFontToken.Body_Text_Bold
            }

            ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: controller.getWindowList()

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
                            text: modelData.ownerName + " - " + modelData.width + "×" + modelData.height
                            fontEnum: UIFontToken.Caption_Text
                            colorEnum: UIColorToken.Content_Secondary
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
            // Refresh folder after capture
            folderModel.folder = ""
            folderModel.folder = controller.screenshotsFolderUrl
        }

        function onCaptureCompleted(filePath) {
            console.log("Screenshot saved:", filePath)
            folderModel.folder = ""
            folderModel.folder = controller.screenshotsFolderUrl
        }

        function onErrorOccurred(message) {
            console.error("Screenshot error:", message)
        }
    }
}
