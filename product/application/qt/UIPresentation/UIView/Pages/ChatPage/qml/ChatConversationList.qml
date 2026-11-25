import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Rectangle {
    id: root
    required property ChatPageController controller
    signal inviteRequested()

    color: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Background, UIColorState.Normal)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        RowLayout {
            Layout.fillWidth: true

            UTText {
                Layout.fillWidth: true
                text: qsTr("Chat")
                fontEnum: UIFontToken.Section_Title
                colorEnum: UIColorToken.Content_Heading
            }

            UTButton {
                appearance: UTButton.Secondary
                radius: 6
                text: qsTr("New group")
                enabled: root.controller.ready
                onClicked: groupEditor.show("", "")
            }
        }

        UTButton {
            appearance: UTButton.Ghost
            fontColorEnum: UIColorToken.Link_Text
            text: qsTr("Accept chat invitation")
            enabled: root.controller.ready
            onClicked: root.inviteRequested()
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            model: root.controller.rows
            ScrollBar.vertical: UTScrollBar {}

            delegate: Loader {
                id: rowLoader
                required property var modelData
                width: ListView.view.width
                sourceComponent: modelData.isGroup ? groupRow : conversationRow

                Component {
                    id: groupRow
                    RowLayout {
                        spacing: 2

                        UTButton {
                            Layout.fillWidth: true
                            appearance: UTButton.Ghost
                            Accessible.name: rowLoader.modelData.name
                            Accessible.expandable: true
                            Accessible.expanded: !rowLoader.modelData.collapsed
                            radius: 6
                            contentItem: UTText {
                                text: (rowLoader.modelData.collapsed ? "▸  " : "▾  ") + rowLoader.modelData.name
                                textFormat: Text.PlainText
                                fontEnum: UIFontToken.Body_Text_Medium
                                colorEnum: UIColorToken.Content_Secondary_Text
                                elide: Text.ElideRight
                                verticalAlignment: Text.AlignVCenter
                            }
                            onClicked: root.controller.toggleGroup(rowLoader.modelData.id)
                        }

                        UTButton {
                            id: groupOptionsButton
                            visible: rowLoader.modelData.id.length > 0
                            appearance: UTButton.Ghost
                            Accessible.name: qsTr("Group options")
                            text: "⋯"
                            onClicked: {
                                groupMenu.groupId = rowLoader.modelData.id
                                groupMenu.groupName = rowLoader.modelData.name
                                groupMenu.popup(groupOptionsButton, 0, height)
                            }
                        }
                    }
                }

                Component {
                    id: conversationRow
                    UTButton {
                        id: conversationButton
                        readonly property bool selected: root.controller.conversation.id === rowLoader.modelData.id
                        Accessible.name: rowLoader.modelData.name
                        appearance: UTButton.Ghost
                        radius: 7
                        leftPadding: 8
                        rightPadding: 8
                        topPadding: 6
                        bottomPadding: 6
                        backgroundColor: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Nav_Item_Background,
                            selected ? UIColorState.Selected : hovered ? UIColorState.Hovered : UIColorState.Normal)
                        contentItem: RowLayout {
                            spacing: 8
                            Rectangle {
                                Layout.preferredWidth: 32
                                Layout.preferredHeight: 32
                                radius: 16
                                color: UTComponentUtil.getPlainUIColor(UIColorToken.Avatar_Person_Background, UIColorState.Normal)
                                UTText {
                                    anchors.centerIn: parent
                                    text: rowLoader.modelData.name.slice(0, 1).toUpperCase()
                                    textFormat: Text.PlainText
                                    colorEnum: UIColorToken.Avatar_Text
                                }
                            }
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2
                                RowLayout {
                                    Layout.fillWidth: true
                                    UTText {
                                        Layout.fillWidth: true
                                        text: rowLoader.modelData.name
                                        textFormat: Text.PlainText
                                        fontEnum: UIFontToken.Body_Text_Medium
                                        elide: Text.ElideRight
                                    }
                                    UTText {
                                        text: rowLoader.modelData.time
                                        fontEnum: UIFontToken.Caption_Text
                                        colorEnum: UIColorToken.Content_Secondary_Text
                                    }
                                }
                                UTText {
                                    Layout.fillWidth: true
                                    text: rowLoader.modelData.preview
                                    textFormat: Text.PlainText
                                    fontEnum: UIFontToken.Caption_Text
                                    colorEnum: UIColorToken.Content_Secondary_Text
                                    elide: Text.ElideRight
                                }
                            }
                        }
                        onClicked: root.controller.selectConversation(rowLoader.modelData.id)
                    }
                }
            }
        }
    }

    UTDynamicMenu {
        id: groupMenu
        property string groupId
        property string groupName
        model: [{text: qsTr("Rename group"), action: "rename"},
                {text: qsTr("Delete group"), action: "delete"}]
        onItemTriggered: action => {
            if (action === "rename") {
                groupEditor.show(groupMenu.groupId, groupMenu.groupName)
            } else if (action === "delete") {
                confirmDelete.groupId = groupMenu.groupId
                confirmDelete.groupName = groupMenu.groupName
                confirmDelete.open()
            }
        }
    }

    Dialog {
        id: groupEditor
        property string groupId
        property bool attempted: false

        function show(id, name) {
            groupId = id
            groupNameField.text = name
            attempted = false
            open()
        }

        function save() {
            if (!groupNameField.text.trim().length)
                return
            attempted = true
            const saved = groupId.length ? root.controller.renameGroup(groupId, groupNameField.text)
                                         : root.controller.createGroup(groupNameField.text)
            if (saved)
                close()
        }

        title: groupId.length ? qsTr("Rename group") : qsTr("New group")
        anchors.centerIn: Overlay.overlay
        width: 340
        modal: true
        padding: 16
        palette.window: UTComponentUtil.getPlainUIColor(UIColorToken.Dialog_Background, UIColorState.Normal)
        palette.windowText: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Heading, UIColorState.Normal)
        background: Rectangle {
            radius: 10
            color: groupEditor.palette.window
            border.color: UTComponentUtil.getPlainUIColor(UIColorToken.Window_Border, UIColorState.Normal)
        }
        onOpened: {
            groupNameField.forceActiveFocus()
            groupNameField.selectAll()
        }

        contentItem: ColumnLayout {
            spacing: 12
            UTTextField {
                id: groupNameField
                Layout.fillWidth: true
                Accessible.name: placeholderText
                placeholderText: qsTr("Group name")
                maximumLength: 64
                onTextEdited: groupEditor.attempted = false
                onAccepted: groupEditor.save()
            }
            UTText {
                Layout.fillWidth: true
                visible: groupEditor.attempted && root.controller.error.length > 0
                text: root.controller.error
                textFormat: Text.PlainText
                wrapMode: Text.Wrap
                colorEnum: UIColorToken.Status_Error_Text
            }
            RowLayout {
                Layout.alignment: Qt.AlignRight
                UTButton {
                    appearance: UTButton.Secondary
                    text: qsTr("Cancel")
                    onClicked: groupEditor.close()
                }
                UTButton {
                    text: groupEditor.groupId.length ? qsTr("Save") : qsTr("Create")
                    enabled: groupNameField.text.trim().length > 0
                    onClicked: groupEditor.save()
                }
            }
        }
    }

    Dialog {
        id: confirmDelete
        property string groupId
        property string groupName
        title: qsTr("Delete group")
        anchors.centerIn: Overlay.overlay
        width: 380
        modal: true
        padding: 16
        palette.window: UTComponentUtil.getPlainUIColor(UIColorToken.Dialog_Background, UIColorState.Normal)
        palette.windowText: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Heading, UIColorState.Normal)
        background: Rectangle {
            radius: 10
            color: confirmDelete.palette.window
            border.color: UTComponentUtil.getPlainUIColor(UIColorToken.Window_Border, UIColorState.Normal)
        }

        contentItem: ColumnLayout {
            spacing: 12
            UTText {
                Layout.fillWidth: true
                text: qsTr("Conversations in “%1” will move to Ungrouped.").arg(confirmDelete.groupName)
                textFormat: Text.PlainText
                wrapMode: Text.Wrap
            }
            RowLayout {
                Layout.alignment: Qt.AlignRight
                UTButton {
                    appearance: UTButton.Secondary
                    text: qsTr("Cancel")
                    onClicked: confirmDelete.close()
                }
                UTButton {
                    appearance: UTButton.Destructive
                    text: qsTr("Delete group")
                    onClicked: {
                        root.controller.deleteGroup(confirmDelete.groupId)
                        confirmDelete.close()
                    }
                }
            }
        }
    }
}
