import QtQuick
import QtQuick.Layouts
import Qt.labs.qmlmodels
import UTComponent 1.0
import "../common"

GalleryCase {
    id: root
    objectName: "utTreeViewCase"

    property var currentDocument: null

    title: "UTTreeView"

    TreeModel {
        id: documentModel
        objectName: "treeDocumentModel"
        TableModelColumn { display: "title" }

        rows: [
            {
                title: qsTr("Guides"),
                folder: true,
                rows: [
                    {
                        title: qsTr("Quick start.md"),
                        path: "/Guides/Quick start.md",
                        format: "Markdown",
                        content: qsTr("Quick start\n\nOpen the workspace, choose a document, and review its contents in this preview.")
                    },
                    {
                        title: qsTr("Reference"),
                        folder: true,
                        rows: [
                            {
                                title: qsTr("Keyboard navigation and document shortcuts.md"),
                                path: "/Guides/Reference/Keyboard navigation and document shortcuts.md",
                                format: "Markdown",
                                content: qsTr("Keyboard navigation\n\nUse the arrow keys to move through the tree. Open a document with Enter or Space. Use the folder indicators to expand and collapse branches.")
                            },
                            {
                                title: qsTr("Document formats.txt"),
                                path: "/Guides/Reference/Document formats.txt",
                                format: "Plain text",
                                content: qsTr("Document formats\n\nMarkdown stores headings and paragraphs. Plain text keeps short notes without formatting.")
                            }
                        ]
                    }
                ]
            },
            {
                title: qsTr("Project notes"),
                folder: true,
                rows: [
                    {
                        title: qsTr("Release checklist.txt"),
                        path: "/Project notes/Release checklist.txt",
                        format: "Plain text",
                        content: qsTr("Release checklist\n\nReview the updated guides. Check the document names. Share the release notes with the project team.")
                    },
                    {
                        title: qsTr("Planning.md"),
                        path: "/Project notes/Planning.md",
                        format: "Markdown",
                        content: qsTr("Planning\n\nThe next iteration adds examples for document organization and keyboard access.")
                    }
                ]
            },
            {
                title: "README.md",
                path: "/README.md",
                format: "Markdown",
                content: qsTr("Document library\n\nGuides contains getting-started and reference material. Project notes contains the release checklist and planning notes.")
            }
        ]
    }

    GalleryDemo {
        title: qsTr("Document library")

        UTTreeView {
            id: documentTree
            objectName: "documentTreeView"
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            Layout.maximumWidth: 520
            Layout.preferredHeight: 200
            model: documentModel
            expandAll: true
            focusOnShow: false

            function activate(index, toggleFolder = false) {
                if (!index.valid)
                    return

                const node = documentModel.getRow(index)
                if (node.folder) {
                    if (toggleFolder)
                        treeView.toggleExpanded(treeView.rowAtIndex(index))
                } else {
                    root.currentDocument = node
                }
            }

            onItemInvoked: function(index) { activate(index, true) }

            delegate: UTTreeViewDelegate {
                objectName: "documentTreeDelegate_" + row
                text: model.display
                highlighted: current
                onClicked: documentTree.activate(treeView.index(row, 0))
            }
        }

        UTLabel {
            objectName: "treeDocumentTitle"
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            Layout.maximumWidth: 520
            text: root.currentDocument ? root.currentDocument.title : qsTr("Document preview")
            wrapMode: Text.Wrap
        }

        UTLabel {
            objectName: "treeDocumentMetadata"
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            Layout.maximumWidth: 520
            visible: root.currentDocument !== null
            text: root.currentDocument
                  ? root.currentDocument.path + "\n" + root.currentDocument.format
                  : ""
            wrapMode: Text.Wrap
        }

        UTTextArea {
            objectName: "treeDocumentContent"
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            Layout.maximumWidth: 520
            Layout.minimumHeight: 132
            readOnly: true
            text: root.currentDocument
                  ? root.currentDocument.content
                  : qsTr("Open a document to read its contents.")
        }
    }
}
