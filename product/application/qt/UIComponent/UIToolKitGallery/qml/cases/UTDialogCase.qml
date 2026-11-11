import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import UTComponent 1.0
import "../common"

GalleryCase {
    id: root
    objectName: "utDialogCase"

    title: "UTDialog"

    property string displayName: qsTr("Alex Morgan")
    property var demoDialog: null
    property bool _destroying: false

    function openDialog() {
        if (demoDialog) {
            demoDialog.raise()
            demoDialog.requestActivate()
            return
        }
        demoDialog = dialogComponent.createObject(root, {
            "transientParent": root.Window.window
        })
    }

    function closeDialog() {
        if (demoDialog)
            demoDialog.close()
    }

    onVisibleChanged: {
        if (!visible)
            closeDialog()
    }
    Component.onDestruction: {
        _destroying = true
        closeDialog()
    }

    Timer {
        id: restoreFocus
        interval: 0
        onTriggered: {
            if (root.visible && !root._destroying) {
                if (root.Window.window)
                    root.Window.window.requestActivate()
                openDialogButton.forceActiveFocus(Qt.OtherFocusReason)
            }
        }
    }

    GalleryDemo {
        title: qsTr("Display name")

        UTTextField {
            objectName: "appliedDisplayNameField"
            Layout.fillWidth: true
            Layout.maximumWidth: 320
            text: root.displayName
            readOnly: true
        }
        UTButton {
            id: openDialogButton
            objectName: "openDialogButton"
            text: qsTr("Edit display name")
            onClicked: root.openDialog()
        }
    }

    Component {
        id: dialogComponent

        UTDialog {
            id: editDialog
            closeOnEscape: true
            objectName: "demoDialog"
            title: qsTr("Edit display name")
            width: 420
            height: 220
            minimumWidth: 360
            minimumHeight: 220
            x: transientParent ? transientParent.x + (transientParent.width - width) / 2 : 0
            y: transientParent ? transientParent.y + (transientParent.height - height) / 2 : 0

            function applyDisplayName() {
                if (!confirmButton.enabled)
                    return
                root.displayName = displayNameEditor.text.trim()
                close()
            }

            onClosing: {
                root.demoDialog = null
                if (root.visible && !root._destroying)
                    restoreFocus.start()
                destroy()
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 24
                spacing: 12

                UTLabel {
                    text: qsTr("Display name")
                }
                UTTextField {
                    id: displayNameEditor
                    objectName: "displayNameEditor"
                    Layout.fillWidth: true
                    text: root.displayName
                    focus: true
                    selectByMouse: true
                    onAccepted: editDialog.applyDisplayName()
                    Component.onCompleted: selectAll()
                }
                Item {
                    Layout.fillHeight: true
                }
                RowLayout {
                    Layout.alignment: Qt.AlignRight
                    spacing: 12

                    UTButton {
                        objectName: "cancelDialogButton"
                        text: qsTr("Cancel")
                        onClicked: editDialog.close()
                    }
                    UTButton {
                        id: confirmButton
                        objectName: "confirmDialogButton"
                        text: qsTr("Confirm")
                        enabled: displayNameEditor.text.trim().length > 0
                        onClicked: editDialog.applyDisplayName()
                    }
                }
            }
        }
    }
}
