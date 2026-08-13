import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * ContactEditDialog - Modal add / rename dialog for the contact tree.
 *
 * A standalone window spawned by ContactsPageController via createQmlWindow().
 * The controller injects itself plus the initial field values; this view only
 * collects input and forwards the result. Centering is handled by the C++ side.
 */
UTDialog {
    id: dialog

    // Injected via createWithInitialProperties() by the controller.
    required property var controller
    required property string mode        // "add" | "edit"
    required property string parentId
    required property string editId
    required property int    nodeType    // 0 = Person, 1 = Group
    required property string initialName

    title: mode === "add"
           ? (nodeType === 1 ? "新增分组" : "新增联系人")
           : "重命名"
    width: 360
    height: contentColumn.implicitHeight + 48
    minimumWidth: 320
    minimumHeight: 150

    function _confirm() {
        const name = nameField.text.trim();
        if (name === "") return;
        if (mode === "add") {
            controller.addContact(parentId, { displayName: name, nodeType: nodeType });
        } else {
            controller.updateContact(editId, { displayName: name });
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
                onClicked: dialog._confirm()
            }
        }
    }
}
