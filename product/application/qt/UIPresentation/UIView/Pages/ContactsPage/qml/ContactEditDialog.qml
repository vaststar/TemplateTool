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
    ListModel {
        id: genderOptions
        ListElement { text: qsTr("Unspecified"); value: 0 }
        ListElement { text: qsTr("Male"); value: 1 }
        ListElement { text: qsTr("Female"); value: 2 }
        ListElement { text: qsTr("Other"); value: 3 }
    }

    title: mode === "add"
           ? (isPerson ? qsTr("Add contact") : qsTr("Add group"))
           : (isPerson ? qsTr("Edit contact") : qsTr("Rename group"))
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
        for (let i = 0; i < genderOptions.count; ++i) {
            if (genderOptions.get(i).value === initialGender) {
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
                id: displayNameLabel
                text: qsTr("Display name")
                Layout.alignment: Qt.AlignVCenter
            }

            UTTextField {
                id: nameField
                Accessible.name: displayNameLabel.text
                Layout.fillWidth: true
                maximumLength: 128
                placeholderText: qsTr("Name")
                onAccepted: dialog._confirm()
            }

            UTText {
                id: firstNameLabel
                text: qsTr("First name")
                visible: dialog.isPerson
                Layout.alignment: Qt.AlignVCenter
            }

            UTTextField {
                id: firstNameField
                Accessible.name: firstNameLabel.text
                Layout.fillWidth: true
                visible: dialog.isPerson
                maximumLength: 64
                onAccepted: dialog._confirm()
            }

            UTText {
                id: lastNameLabel
                text: qsTr("Last name")
                visible: dialog.isPerson
                Layout.alignment: Qt.AlignVCenter
            }

            UTTextField {
                id: lastNameField
                Accessible.name: lastNameLabel.text
                Layout.fillWidth: true
                visible: dialog.isPerson
                maximumLength: 64
                onAccepted: dialog._confirm()
            }

            UTText {
                id: genderLabel
                text: qsTr("Gender")
                visible: dialog.isPerson
                Layout.alignment: Qt.AlignVCenter
            }

            UTComboBox {
                id: genderField
                Accessible.name: genderLabel.text + ": " + genderField.displayText
                Layout.fillWidth: true
                visible: dialog.isPerson
                model: genderOptions
                textRole: "text"
                valueRole: "value"
                currentIndex: 0
            }

            UTText {
                id: phoneLabel
                text: qsTr("Phone")
                visible: dialog.isPerson
                Layout.alignment: Qt.AlignVCenter
            }

            UTTextField {
                id: phoneField
                Accessible.name: phoneLabel.text
                Layout.fillWidth: true
                visible: dialog.isPerson
                maximumLength: 64
                inputMethodHints: Qt.ImhDialableCharactersOnly
                onAccepted: dialog._confirm()
            }

            UTText {
                id: emailLabel
                text: qsTr("Email")
                visible: dialog.isPerson
                Layout.alignment: Qt.AlignVCenter
            }

            UTTextField {
                id: emailField
                Accessible.name: emailLabel.text
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
