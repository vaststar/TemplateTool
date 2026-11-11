import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import UTComponent 1.0
import UTComposite 1.0
import "../common"

GalleryCase {
    id: root
    objectName: "utMessageDialogCase"

    title: "UTMessageDialog"

    property bool notificationsEnabled: false
    property var messageDialog: null
    property string _preset: ""
    property var _returnFocus: null
    property bool _destroying: false

    function openMessage(preset, button) {
        if (messageDialog) {
            messageDialog.raise()
            messageDialog.requestActivate()
            return
        }
        const dialog = messageComponent.createObject(root, {
            "transientParent": root.Window.window
        })
        if (!dialog)
            return
        messageDialog = dialog
        _preset = preset
        _returnFocus = button
        if (!GalleryFixtures.configureMessageDialog(dialog, preset)) {
            releaseMessage(false)
            return
        }
        dialog.show()
        dialog.requestActivate()
    }

    function releaseMessage(restore) {
        disposeDialog.stop()
        const dialog = messageDialog
        messageDialog = null
        _preset = ""
        if (dialog) {
            if (dialog.visible)
                dialog.close()
            dialog.destroy()
        }
        if (restore && root.visible && !root._destroying)
            restoreFocus.start()
    }

    onVisibleChanged: {
        if (!visible)
            releaseMessage(false)
    }
    Component.onDestruction: {
        _destroying = true
        releaseMessage(false)
    }

    Connections {
        target: GalleryFixtures
        function onMessageClosed(dialog, buttonIndex, buttonText, accepted) {
            if (dialog !== root.messageDialog)
                return
            if (accepted) {
                if (root._preset === "confirm")
                    root.notificationsEnabled = !root.notificationsEnabled
                else if (root._preset === "reset")
                    root.notificationsEnabled = false
            }
            disposeDialog.start()
        }
    }

    Connections {
        target: root.messageDialog
        function onVisibleChanged() {
            if (root.messageDialog && !root.messageDialog.visible)
                disposeDialog.start()
        }
    }

    // UTMessageDialog closes from its controller's result signal. Dispose on
    // the next event turn so the Gallery can consume that same result first.
    Timer {
        id: disposeDialog
        interval: 0
        onTriggered: root.releaseMessage(true)
    }
    Timer {
        id: restoreFocus
        interval: 0
        onTriggered: {
            if (root.visible && !root._destroying && root._returnFocus) {
                if (root.Window.window)
                    root.Window.window.requestActivate()
                const button = root._returnFocus.enabled ? root._returnFocus : confirmationButton
                button.forceActiveFocus(Qt.OtherFocusReason)
            }
        }
    }

    GalleryDemo {
        title: qsTr("Information")

        UTButton {
            id: informationButton
            objectName: "openInformationButton"
            text: qsTr("Report ready")
            onClicked: root.openMessage("info", informationButton)
        }
    }

    GalleryDemo {
        title: qsTr("Confirmation")

        UTLabel {
            objectName: "notificationPreference"
            Layout.fillWidth: true
            text: root.notificationsEnabled ? qsTr("Notifications on") : qsTr("Notifications off")
            wrapMode: Text.Wrap
        }
        UTButton {
            id: confirmationButton
            objectName: "openConfirmationButton"
            text: qsTr("Change preference")
            onClicked: root.openMessage("confirm", confirmationButton)
        }
    }

    GalleryDemo {
        title: qsTr("Destructive confirmation")

        UTButton {
            id: resetButton
            objectName: "openResetButton"
            text: qsTr("Reset preference")
            enabled: root.notificationsEnabled
            onClicked: root.openMessage("reset", resetButton)
        }
    }

    Component {
        id: messageComponent

        UTMessageDialog {
            objectName: "demoMessageDialog"
            x: transientParent ? transientParent.x + (transientParent.width - width) / 2 : 0
            y: transientParent ? transientParent.y + (transientParent.height - height) / 2 : 0
        }
    }
}
