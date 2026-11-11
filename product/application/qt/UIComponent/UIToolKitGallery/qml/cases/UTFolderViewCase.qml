import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import UTComposite 1.0
import UIResourceLoader 1.0
import "../common"

GalleryCase {
    id: root
    objectName: "utFolderViewCase"

    property url sampleFolderUrl: GalleryFixtures.folderUrl
    property url emptyFolderUrl: GalleryFixtures.emptyFolderUrl
    property var openedDocument: null
    readonly property var selectedDocument: documentFolder.selectedFilePath.length > 0
                                            ? documentForPath(documentFolder.selectedFilePath) : null

    title: "UTFolderView"

    function documentForPath(path) {
        const normalizedPath = String(path).replace(/\\/g, "/")
        const name = normalizedPath.substring(normalizedPath.lastIndexOf("/") + 1)
        const extension = name.indexOf(".") >= 0 ? name.split(".").pop().toUpperCase() : ""
        const sample = sampleDocuments[name]
        return {
            title: name,
            path: normalizedPath,
            format: extension === "MD" ? qsTr("Markdown") : qsTr("Plain text"),
            summary: sample ? sample.summary : qsTr("A document in the sample library."),
            content: sample ? sample.content : name + "\n\n" + qsTr("A sample document in the Gallery library.")
        }
    }

    readonly property var sampleDocuments: ({
        "README.md": {
            summary: qsTr("An introduction to the sample document library."),
            content: qsTr("Document library\n\nUse the grid to browse documents, or switch to the detail view to compare names, modification dates, sizes, and file types.")
        },
        "Release checklist.txt": {
            summary: qsTr("Tasks to complete before publishing a release."),
            content: qsTr("Release checklist\n\nReview the updated guides. Check the document names. Share the release notes with the project team.")
        },
        "Keyboard navigation and document shortcuts.md": {
            summary: qsTr("A reference for browsing documents with the keyboard."),
            content: qsTr("Keyboard navigation\n\nUse the arrow keys to move between documents. Press Enter or Space to open the document menu, then choose Open to read the document.")
        }
    })

    Component {
        id: documentThumbnail

        ColumnLayout {
            spacing: 4

            UTLabel {
                Layout.alignment: Qt.AlignHCenter
                text: fileName.split(".").pop().toUpperCase()
                fontEnum: UIFontToken.Heading_Text
                colorEnum: UIColorToken.Content_Secondary_Text
            }
            UTLabel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: 0
                Layout.minimumHeight: 0
                text: root.documentForPath(fileName).summary
                fontEnum: UIFontToken.Caption_Text
                colorEnum: UIColorToken.Content_Secondary_Text
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignTop
                wrapMode: Text.Wrap
                maximumLineCount: 3
                elide: Text.ElideRight
            }
        }
    }

    GalleryDemo {
        title: qsTr("Document library")

        UTFolderView {
            id: documentFolder
            objectName: "documentFolderView"
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            Layout.maximumWidth: 600
            Layout.preferredHeight: 240
            folderUrl: root.sampleFolderUrl
            nameFilters: ["*.md", "*.txt"]
            gridThumbnail: documentThumbnail
            onFileOpenRequested: function(filePath) {
                root.openedDocument = root.documentForPath(filePath)
            }
        }

        ColumnLayout {
            objectName: "folderDocumentSelectionDetails"
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            Layout.maximumWidth: 600
            spacing: 4

            UTLabel {
                objectName: "folderSelectedDocumentTitle"
                Layout.fillWidth: true
                Layout.minimumWidth: 0
                text: root.selectedDocument ? root.selectedDocument.title : qsTr("Select a document")
                fontEnum: UIFontToken.Body_Text_Medium
                wrapMode: Text.Wrap
            }
            UTLabel {
                objectName: "folderSelectedDocumentMetadata"
                Layout.fillWidth: true
                Layout.minimumWidth: 0
                visible: root.selectedDocument !== null
                text: root.selectedDocument
                      ? root.selectedDocument.format + "\n" + root.selectedDocument.summary : ""
                wrapMode: Text.Wrap
            }
            UTLabel {
                objectName: "folderSelectedDocumentPath"
                Layout.fillWidth: true
                Layout.minimumWidth: 0
                visible: root.selectedDocument !== null
                text: root.selectedDocument ? root.selectedDocument.path : ""
                colorEnum: UIColorToken.Content_Secondary_Text
                wrapMode: Text.WrapAnywhere
            }
        }

        UTLabel {
            objectName: "folderOpenedDocumentTitle"
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            Layout.maximumWidth: 600
            text: root.openedDocument ? root.openedDocument.title : qsTr("Document preview")
            fontEnum: UIFontToken.Body_Text_Medium
            wrapMode: Text.Wrap
        }

        UTTextArea {
            objectName: "folderOpenedDocumentContent"
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            Layout.maximumWidth: 600
            Layout.minimumHeight: 132
            readOnly: true
            text: root.openedDocument
                  ? root.openedDocument.content : qsTr("Open a document to read its contents.")
        }
    }

    GalleryDemo {
        title: qsTr("Empty document library")

        UTFolderView {
            objectName: "emptyDocumentFolderView"
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            Layout.maximumWidth: 600
            Layout.preferredHeight: 240
            folderUrl: root.emptyFolderUrl
            emptyTitle: qsTr("No documents yet")
            emptyHint: qsTr("Your library is ready.")
            statusTemplate: qsTr("%1 documents")
        }
    }
}
