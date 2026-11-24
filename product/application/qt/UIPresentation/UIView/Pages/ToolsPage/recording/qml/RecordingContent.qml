import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UTComponent
import UTComposite
import UIView 1.0
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
            folderView.scrollToFile(filePath)
            controller.requestThumbnail(filePath)
        }

        function onFileDeleted(filePath) {
            // FolderListModel watches the directory and drops the entry on its
            // own; just clear the now-stale selection (no full reload needed).
            folderView.clearSelection()
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
                folderView.scrollToFile(filePath)
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
    property var thumbnailCache: ({})

    // Custom video thumbnail for grid view
    Component {
        id: videoGridThumbnail
        Rectangle {
            id: thumbnailRoot
            color: UTComponentUtil.getPlainUIColor(UIColorToken.Media_Background, UIColorState.Normal)
            radius: 4

            property string currentFilePath: (typeof filePath !== "undefined" && filePath) ? filePath : ""
            property string thumbnailUrl: ""

            function refreshThumbnail() {
                if (!currentFilePath || currentFilePath.length === 0)
                    return

                var cached = root.thumbnailCache[currentFilePath]
                if (cached && cached.length > 0) {
                    thumbnailUrl = cached
                    return
                }

                var readyUrl = controller.getThumbnailUrl(currentFilePath)
                if (readyUrl && readyUrl.length > 0) {
                    var newCache = root.thumbnailCache
                    newCache[currentFilePath] = readyUrl
                    root.thumbnailCache = newCache
                    thumbnailUrl = readyUrl
                    return
                }

                thumbnailUrl = ""
                loadTimer.restart()
            }

            Component.onCompleted: refreshThumbnail()
            onCurrentFilePathChanged: refreshThumbnail()

            Connections {
                target: controller

                function onThumbnailReady(videoPath, readyUrl) {
                    if (videoPath === thumbnailRoot.currentFilePath) {
                        var newCache = root.thumbnailCache
                        newCache[thumbnailRoot.currentFilePath] = readyUrl
                        root.thumbnailCache = newCache
                        thumbnailRoot.thumbnailUrl = readyUrl
                    }
                }
            }

            Timer {
                id: loadTimer
                interval: 30
                repeat: false
                onTriggered: controller.requestThumbnail(thumbnailRoot.currentFilePath)
            }

            Image {
                anchors.fill: parent
                source: thumbnailRoot.thumbnailUrl
                fillMode: Image.PreserveAspectCrop
                asynchronous: true
                cache: true
                smooth: true
                visible: source.toString().length > 0
            }

            Text {
                visible: thumbnailRoot.thumbnailUrl.length === 0
                anchors.centerIn: parent
                text: "🎥"
                // emoji glyph size, intentionally not tokenized
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
                color: Qt.alpha(UTComponentUtil.getPlainUIColor(UIColorToken.Media_Scrim, UIColorState.Normal), 0.267)

                Text {
                    id: _fmtLabel
                    anchors.centerIn: parent
                    text: {
                        var ext = fileName.split('.').pop()
                        return ext ? ext.toUpperCase() : ""
                    }
                    font: UTComponentUtil.getUIFont(UIFontToken.Badge_Text)
                    color: UTComponentUtil.getPlainUIColor(UIColorToken.Media_Text, UIColorState.Normal)
                }
            }
        }
    }

    // Custom video icon for detail view
    Component {
        id: videoDetailIcon
        Rectangle {
            id: detailRoot
            width: 18
            height: 18
            radius: 3
            color: UTComponentUtil.getPlainUIColor(UIColorToken.Media_Background, UIColorState.Normal)

            property string currentFilePath: (typeof filePath !== "undefined" && filePath) ? filePath : ""
            property string thumbnailUrl: ""

            function refreshThumbnail() {
                if (!currentFilePath || currentFilePath.length === 0)
                    return

                var cached = root.thumbnailCache[currentFilePath]
                if (cached && cached.length > 0) {
                    thumbnailUrl = cached
                    return
                }

                var readyUrl = controller.getThumbnailUrl(currentFilePath)
                if (readyUrl && readyUrl.length > 0) {
                    var newCache = root.thumbnailCache
                    newCache[currentFilePath] = readyUrl
                    root.thumbnailCache = newCache
                    thumbnailUrl = readyUrl
                    return
                }

                thumbnailUrl = ""
                detailLoadTimer.restart()
            }

            Component.onCompleted: refreshThumbnail()
            onCurrentFilePathChanged: refreshThumbnail()

            Connections {
                target: controller

                function onThumbnailReady(videoPath, readyUrl) {
                    if (videoPath === detailRoot.currentFilePath) {
                        var newCache = root.thumbnailCache
                        newCache[detailRoot.currentFilePath] = readyUrl
                        root.thumbnailCache = newCache
                        detailRoot.thumbnailUrl = readyUrl
                    }
                }
            }

            Timer {
                id: detailLoadTimer
                interval: 30
                repeat: false
                onTriggered: controller.requestThumbnail(detailRoot.currentFilePath)
            }

            Image {
                anchors.fill: parent
                source: detailRoot.thumbnailUrl
                fillMode: Image.PreserveAspectCrop
                asynchronous: true
                cache: true
                smooth: true
                visible: source.toString().length > 0
            }

            Text {
                visible: detailRoot.thumbnailUrl.length === 0
                anchors.centerIn: parent
                text: "🎥"
                // emoji glyph size, intentionally not tokenized
                font.pixelSize: 14
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
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
                appearance: UTButton.Primary
                id: startBtn
                text: qsTr("🎬 Start Recording")
                focus: true
                enabled: controller.ffmpegAvailable && !controller.isRecording && !root.controller.screenPermission.busy

                onClicked: {
                    if (!controller.canStartRecording()) return
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
                Accessible.name: qsTr("Recording mode") + ": " + modeCombo.displayText
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
                color: UTComponentUtil.getPlainUIColor(UIColorToken.Status_Error_Text, UIColorState.Normal)
            }

            Item { Layout.fillWidth: true }

            UTButton {
                appearance: UTButton.Secondary
                text: qsTr("📂 Open Folder")
                onClicked: controller.openRecordingsFolder()
            }
        }

        PermissionGuide {
            Layout.fillWidth: true
            controller: root.controller.screenPermission
            message: qsTr("Screen recording access is required to record your screen.")
        }

        // === Error Banner ===
        UTText {
            id: errorText
            visible: false
            Layout.fillWidth: true
            color: UTComponentUtil.getPlainUIColor(UIColorToken.Status_Error_Text, UIColorState.Normal)
            fontEnum: UIFontToken.Caption_Text
            wrapMode: Text.WordWrap
        }

        // === Folder View ===
        UTFolderView {
            id: folderView
            Layout.fillWidth: true
            Layout.fillHeight: true

            folderUrl: controller.recordingsFolderUrl
            canCopy: false // RecordingController has no clipboard action.
            nameFilters: ["*.mp4", "*.webm", "*.mov", "*.mkv", "*.avi", "*.gif"]

            gridThumbnail: videoGridThumbnail
            detailIcon: videoDetailIcon

            emptyIcon: "🎬"
            emptyTitle: qsTr("No recordings yet")
            emptyHint: qsTr("Click 'Start Recording' to capture your screen")
            statusTemplate: qsTr("%1 recordings")

            onFileOpenRequested: (fp) => controller.openFile(fp)
            onFileRevealRequested: (fp) => controller.openRecordingsFolder()
            onFileDeleteRequested: (fp) => controller.deleteFile(fp)

            statusBarExtra: Component {
                RowLayout {
                    spacing: 8

                    UTButton {
                        appearance: UTButton.Secondary
                        visible: folderView.selectedFilePath.length > 0
                        text: qsTr("▶ Open")
                        onClicked: controller.openFile(folderView.selectedFilePath)
                    }

                    UTButton {
                        appearance: UTButton.Destructive
                        visible: folderView.selectedFilePath.length > 0
                        text: qsTr("🗑 Delete")
                        onClicked: controller.deleteFile(folderView.selectedFilePath)
                    }
                }
            }
        }
    }
}
