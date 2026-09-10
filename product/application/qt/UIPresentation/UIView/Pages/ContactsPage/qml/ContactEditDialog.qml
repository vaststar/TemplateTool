import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * ContactEditDialog - Modal add / edit dialog for the contact tree.
 *
 * Person contacts expose their full profile. Group contacts continue to expose
 * only the display name.
 */
UTDialog {
    id: dialog

    required property var controller
    required property string mode        // "add" | "edit"
    required property string parentId
    required property string editId
    required property int nodeType       // 0 = Person, 1 = Group
    required property var initialInfo

    readonly property bool isPerson: nodeType === 0
    readonly property bool formValid: nameField.text.trim() !== ""
    readonly property var genderOptions: [
        { text: "未指定", value: 0 },
        { text: "男", value: 1 },
        { text: "女", value: 2 },
        { text: "其他", value: 3 }
    ]

    title: mode === "add"
           ? (isPerson ? "新增联系人" : "新增分组")
           : (isPerson ? "编辑联系人" : "重命名分组")
    width: 440
    height: contentColumn.implicitHeight + 48
    minimumWidth: 400
    minimumHeight: 150

    function _fields() {
        const fields = {
            displayName: nameField.text.trim(),
            nodeType: nodeType
        }

        if (isPerson) {
            fields.firstName = firstNameField.text.trim()
            fields.lastName = lastNameField.text.trim()
            fields.gender = genderField.currentValue
            fields.phone = phoneField.text.trim()
            fields.email = emailField.text.trim()
        }

        return fields
    }

    function _confirm() {
        if (!formValid)
            return

        const fields = _fields()

        if (mode === "add")
            controller.addContact(parentId, fields)
        else
            controller.updateContact(editId, fields)

        close()
    }

    Component.onCompleted: {
        const info = initialInfo || {}
        const person = info.person || {}

        nameField.text = info.name || ""
        firstNameField.text = person.firstName || ""
        lastNameField.text = person.lastName || ""
        phoneField.text = person.phone || ""
        emailField.text = person.email || ""

        const initialGender = Number(person.genderValue ?? 0)
        for (let i = 0; i < genderOptions.length; ++i) {
            if (genderOptions[i].value === initialGender) {
                genderField.currentIndex = i
                break
            }
        }

        nameField.forceActiveFocus()
        nameField.selectAll()
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

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 12
            rowSpacing: 12

            UTText {
                text: "显示名称"
                Layout.alignment: Qt.AlignVCenter
            }

            UTTextField {
                id: nameField
                Layout.fillWidth: true
                maximumLength: 128
                placeholderText: "名称"
                onAccepted: dialog._confirm()
            }

            UTText {
                text: "名"
                visible: dialog.isPerson
                Layout.alignment: Qt.AlignVCenter
            }

            UTTextField {
                id: firstNameField
                Layout.fillWidth: true
                visible: dialog.isPerson
                maximumLength: 64
                onAccepted: dialog._confirm()
            }

            UTText {
                text: "姓"
                visible: dialog.isPerson
                Layout.alignment: Qt.AlignVCenter
            }

            UTTextField {
                id: lastNameField
                Layout.fillWidth: true
                visible: dialog.isPerson
                maximumLength: 64
                onAccepted: dialog._confirm()
            }

            UTText {
                text: "性别"
                visible: dialog.isPerson
                Layout.alignment: Qt.AlignVCenter
            }

            UTComboBox {
                id: genderField
                Layout.fillWidth: true
                visible: dialog.isPerson
                model: dialog.genderOptions
                textRole: "text"
                valueRole: "value"
                currentIndex: 0
            }

            UTText {
                text: "电话"
                visible: dialog.isPerson
                Layout.alignment: Qt.AlignVCenter
            }

            UTTextField {
                id: phoneField
                Layout.fillWidth: true
                visible: dialog.isPerson
                maximumLength: 64
                inputMethodHints: Qt.ImhDialableCharactersOnly
                onAccepted: dialog._confirm()
            }

            UTText {
                text: "邮箱"
                visible: dialog.isPerson
                Layout.alignment: Qt.AlignVCenter
            }

            UTTextField {
                id: emailField
                Layout.fillWidth: true
                visible: dialog.isPerson
                maximumLength: 254
                inputMethodHints: Qt.ImhEmailCharactersOnly
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
                enabled: dialog.formValid
                onClicked: dialog._confirm()
            }
        }
    }
}
