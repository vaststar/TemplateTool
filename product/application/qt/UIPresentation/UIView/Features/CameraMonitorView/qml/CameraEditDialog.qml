import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * CameraEditDialog - Modal add / rename dialog for the camera directory tree.
 *
 * A standalone window spawned by CameraMonitorViewController via createQmlWindow().
 * The controller injects itself plus the initial field values; this view only
 * collects input and forwards the result. Centering is handled by the C++ side.
 *
 * Modes:
 *   "add_group"  -> name only, calls controller.addGroup(parentId, {displayName})
 *   "add_camera" -> name + (minimal) source, calls controller.addCamera(parentId, {...})
 *   "edit"       -> rename only, calls controller.updateNode(editId, {displayName})
 */
UTDialog {
    id: dialog

    // Injected via createWithInitialProperties() by the controller.
    required property var controller
    required property string mode        // "add_group" | "add_camera" | "edit"
    required property string parentId
    required property string editId
    required property int    nodeType    // 0 = Group, 1 = Camera
    required property string initialName

    readonly property bool isAddCamera: mode === "add_camera"
    readonly property bool formValid: nameField.text.trim() !== ""
                                      && (!isAddCamera
                                          || sourceKindBox.currentIndex === 0
                                          || urlField.text.trim() !== "")

    title: {
        if (mode === "add_group") return qsTr("Add group");
        if (mode === "add_camera") return qsTr("Add camera");
        return qsTr("Rename");
    }
    width: 360
    height: contentColumn.implicitHeight + 48
    minimumWidth: 320
    minimumHeight: 150

    function _confirm() {
        const name = nameField.text.trim();
        if (!formValid) return;
        if (mode === "add_group") {
            controller.addGroup(parentId, { displayName: name });
        } else if (mode === "add_camera") {
            const isNetwork = sourceKindBox.currentIndex === 1;
            controller.addCamera(parentId, {
                displayName: name,
                sourceKind: isNetwork ? "network" : "local",
                index: localIndexBox.value,
                url: urlField.text.trim()
            });
        } else {
            controller.updateNode(editId, { displayName: name });
        }
        close();
    }

    Component.onCompleted: {
        nameField.text = initialName;
        nameField.forceActiveFocus();
        nameField.selectAll();
    }

    Shortcut {
        sequence: "Escape"
        onActivated: dialog.close()
    }

    ColumnLayout {
        id: contentColumn
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        UTTextField {
            id: nameField
            Accessible.name: placeholderText
            Layout.fillWidth: true
            placeholderText: qsTr("Name")
            onAccepted: dialog._confirm()
        }

        // ---- Camera source (only when adding a camera) ----
        RowLayout {
            Layout.fillWidth: true
            visible: dialog.isAddCamera
            spacing: 8

            UTComboBox {
                id: sourceKindBox
                Accessible.name: qsTr("Camera source") + ": " + sourceKindBox.displayText
                Layout.preferredWidth: 110
                textRole: "text"
                model: ListModel {
                    ListElement { text: qsTr("Local") }
                    ListElement { text: qsTr("Network") }
                }
                currentIndex: 0
            }

            UTSpinBox {
                id: localIndexBox
                Accessible.name: qsTr("Local camera index")
                Layout.fillWidth: true
                visible: dialog.isAddCamera && sourceKindBox.currentIndex === 0
                from: 0
                to: 16
                value: 0
            }

            UTTextField {
                id: urlField
                Accessible.name: qsTr("Camera URL")
                Layout.fillWidth: true
                visible: dialog.isAddCamera && sourceKindBox.currentIndex === 1
                placeholderText: qsTr("rtsp:// or http:// URL")
                onAccepted: dialog._confirm()
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: 8

            UTButton {
                text: qsTr("Cancel")
                appearance: UTButton.Secondary
                implicitWidth: 72
                implicitHeight: 30
                onClicked: dialog.close()
            }
            UTButton {
                text: qsTr("OK")
                appearance: UTButton.Primary
                implicitWidth: 72
                implicitHeight: 30
                enabled: dialog.formValid
                onClicked: dialog._confirm()
            }
        }
    }
}
