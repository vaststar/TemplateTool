import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UTComponent
import UTComposite
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
            folderView.scrollToFile(filePath)
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

        function onFileDeleted(filePath) {
            // FolderListModel watches the directory and drops the entry on its
            // own; just clear the now-stale selection (no full reload needed).
            folderView.clearSelection()
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

            Item { Layout.fillWidth: true }
        }

        // === Folder View ===
        UTFolderView {
            id: folderView
            Layout.fillWidth: true
            Layout.fillHeight: true

            folderUrl: controller.screenshotsFolderUrl
            nameFilters: ["*.png", "*.jpg", "*.jpeg", "*.bmp", "*.gif"]

            emptyIcon: "\ud83d\udcf7"
            emptyTitle: qsTr("No screenshots yet")
            emptyHint: qsTr("Click 'Capture Region' or 'Full Screen' to take a screenshot")
            statusTemplate: qsTr("%1 screenshots")


            onFileOpenRequested: (fp) => controller.openFile(fp)
            onFileCopyRequested: (fp) => controller.copyFileToClipboard(fp)
            onFileRevealRequested: (fp) => controller.revealInFinder(fp)
            onFileDeleteRequested: (fp) => controller.deleteFile(fp)
        }
    }
}
