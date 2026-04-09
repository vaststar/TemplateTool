import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UTComponent
import UTComposite
import UIResourceLoader 1.0

/**
 * Recording Content - Gallery of recorded videos with capture controls
 *
 * Uses UTFolderView for file browsing with grid/list/detail views.
 */
FocusScope {
    id: root

    required property var controller

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

    Component.onCompleted: folderView.refresh()
    onVisibleChanged: { if (visible) folderView.refresh() }

    Connections {
        target: controller

        function onRecordingCompleted(filePath) {
            folderView.scrollToLatest()
        }

        function onErrorOccurred(message) {
            errorText.text = message
            errorText.visible = true
            errorHideTimer.restart()
            pendingFullscreenBar = false
        }

        function onRecordingStateChanged() {
            if (controller.isRecording && pendingFullscreenBar && !activeFloatingBar) {
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
        id: errorHideTimer
        interval: 5000
        onTriggered: errorText.visible = false
    }

    // Region selector component
    Component {
        id: regionSelectorComponent
        RecordingRegionSelector {
            controller: root.controller
            onRecordingFinished: function(filePath) {
                root.activeRegionSelector = null
                folderView.scrollToLatest()
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

    property var activeFloatingBar: null
    property var activeRegionSelector: null
    property bool pendingFullscreenBar: false

    // Custom video thumbnail for grid view
    Component {
        id: videoGridThumbnail
        Rectangle {
            color: "#222222"
            radius: 4

            Text {
                anchors.centerIn: parent
                text: "🎥"
                font.pixelSize: 32
                opacity: 0.6
            }

            Rectangle {
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 4
                width: _fmtLabel.width + 8
                height: _fmtLabel.height + 4
                radius: 3
                color: "#44000000"

                Text {
                    id: _fmtLabel
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
    }

    // Custom video icon for detail view
    Component {
        id: videoDetailIcon
        Text {
            text: "🎥"
            font.pixelSize: 14
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
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
                id: startBtn
                text: qsTr("🎬 Start Recording")
                focus: true
                enabled: controller.ffmpegAvailable && !controller.isRecording

                onClicked: {
                    if (modeCombo.currentValue === "region") {
                        activeRegionSelector = regionSelectorComponent.createObject(null)
                        if (Qt.platform.os === "osx") {
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

            UTComboBox {
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

        // === Folder View ===
        UTFolderView {
            id: folderView
            Layout.fillWidth: true
            Layout.fillHeight: true

            folderUrl: controller.recordingsFolderUrl
            nameFilters: ["*.mp4", "*.webm", "*.mov", "*.mkv", "*.avi"]

            gridThumbnail: videoGridThumbnail
            detailIcon: videoDetailIcon

            emptyIcon: "🎬"
            emptyTitle: qsTr("No recordings yet")
            emptyHint: qsTr("Click 'Start Recording' to capture your screen")
            statusTemplate: qsTr("%1 recordings")
            deleteDialogTitle: qsTr("Delete Recording")
            deleteDialogMessage: qsTr("Are you sure you want to delete this recording?")

            onFileOpenRequested: (fp) => controller.openFile(fp)
            onFileRevealRequested: (fp) => controller.openRecordingsFolder()
            onFileDeleteRequested: (fp) => controller.deleteFile(fp)

            statusBarExtra: Component {
                RowLayout {
                    spacing: 8

                    UTButton {
                        visible: folderView.selectedFilePath.length > 0
                        text: qsTr("▶ Open")
                        onClicked: controller.openFile(folderView.selectedFilePath)
                    }

                    UTButton {
                        visible: folderView.selectedFilePath.length > 0
                        text: qsTr("🗑 Delete")
                        onClicked: {
                            controller.deleteFile(folderView.selectedFilePath)
                            folderView.clearSelection()
                            folderView.refresh()
                        }
                    }
                }
            }
        }
    }
}
