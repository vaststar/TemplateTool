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

    title: {
        if (mode === "add_group") return "新增分组";
        if (mode === "add_camera") return "新增摄像头";
        return "重命名";
    }
    width: 360
    height: contentColumn.implicitHeight + 48
    minimumWidth: 320
    minimumHeight: 150

    function _confirm() {
        const name = nameField.text.trim();
        if (name === "") return;
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
            Layout.fillWidth: true
            placeholderText: "名称"
            onAccepted: dialog._confirm()
        }

        // ---- Camera source (only when adding a camera) ----
        RowLayout {
            Layout.fillWidth: true
            visible: dialog.isAddCamera
            spacing: 8

            UTComboBox {
                id: sourceKindBox
                Layout.preferredWidth: 110
                model: ["本地", "网络"]
                currentIndex: 0
            }

            UTSpinBox {
                id: localIndexBox
                Layout.fillWidth: true
                visible: dialog.isAddCamera && sourceKindBox.currentIndex === 0
                from: 0
                to: 16
                value: 0
            }

            UTTextField {
                id: urlField
                Layout.fillWidth: true
                visible: dialog.isAddCamera && sourceKindBox.currentIndex === 1
                placeholderText: "rtsp:// 或 http:// 地址"
                onAccepted: dialog._confirm()
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: 8

            UTButton {
                text: "取消"
                backgroundColorEnum: UIColorToken.Content_Background
                borderColorEnum: UIColorToken.Content_Input_Border
                fontColorEnum: UIColorToken.Content_Text
                implicitWidth: 72
                implicitHeight: 30
                onClicked: dialog.close()
            }
            UTButton {
                text: "确定"
                implicitWidth: 72
                implicitHeight: 30
                enabled: nameField.text.trim() !== ""
                         && (!dialog.isAddCamera
                             || sourceKindBox.currentIndex === 0
                             || urlField.text.trim() !== "")
                onClicked: dialog._confirm()
            }
        }
    }
}
