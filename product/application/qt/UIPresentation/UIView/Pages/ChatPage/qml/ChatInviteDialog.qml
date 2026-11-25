import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Dialog {
    id: root
    required property ChatPageController controller
    property bool acceptingAnswer: false
    property bool attempted: false

    function show(answer) {
        acceptingAnswer = answer
        attempted = false
        code.clear()
        linkContact.checked = false
        open()
    }

    function submit() {
        if (!code.text.trim().length)
            return
        attempted = true
        const accepted = acceptingAnswer ? controller.applyAnswer(code.text)
                                         : controller.acceptInvite(code.text, linkContact.visible && linkContact.checked)
        if (accepted)
            close()
    }

    title: acceptingAnswer ? qsTr("Paste a reply") : qsTr("Accept a chat invitation")
    anchors.centerIn: Overlay.overlay
    width: Math.min(500, Overlay.overlay ? Overlay.overlay.width - 28 : 500)
    modal: true
    padding: 16
    palette.window: UTComponentUtil.getPlainUIColor(UIColorToken.Dialog_Background, UIColorState.Normal)
    palette.windowText: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Heading, UIColorState.Normal)
    onOpened: code.forceActiveFocus()
    onClosed: code.clear()

    background: Rectangle {
        radius: 10
        color: root.palette.window
        border.color: UTComponentUtil.getPlainUIColor(UIColorToken.Window_Border, UIColorState.Normal)
    }

    contentItem: ColumnLayout {
        spacing: 10

        UTText {
            Layout.fillWidth: true
            text: root.acceptingAnswer
                ? qsTr("Paste the reply you received from your contact.")
                : qsTr("Paste an invitation you were expecting. You can verify the other device before sending messages.")
            wrapMode: Text.WordWrap
        }

        UTTextArea {
            id: code
            Layout.fillWidth: true
            Layout.preferredHeight: 108
            Accessible.name: placeholderText
            placeholderText: root.acceptingAnswer ? qsTr("Reply text") : qsTr("Invitation text")
            textFormat: TextEdit.PlainText
            wrapMode: TextEdit.WrapAnywhere
            onTextChanged: root.attempted = false
        }

        UTCheckBox {
            id: linkContact
            Layout.fillWidth: true
            visible: !root.acceptingAnswer && root.controller.conversation.isContact === true
                     && root.controller.conversation.isVerified === false
            text: qsTr("Link this invitation to %1").arg(root.controller.conversation.name || "")
            Accessible.name: text
            contentItem: Text {
                text: linkContact.text
                textFormat: Text.PlainText
                font: linkContact.textFont
                color: linkContact.textColor
                verticalAlignment: Text.AlignVCenter
                leftPadding: linkContact.indicator.width + linkContact.spacing
                wrapMode: Text.Wrap
            }
        }

        UTText {
            Layout.fillWidth: true
            visible: root.attempted
            text: (root.acceptingAnswer ? root.controller.connectionError : root.controller.error)
                  || root.controller.error
                  || qsTr("Check the text and try again.")
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
            colorEnum: UIColorToken.Status_Error_Text
        }

        RowLayout {
            Layout.fillWidth: true
            UTButton {
                appearance: UTButton.Secondary
                text: qsTr("Paste")
                enabled: code.canPaste
                onClicked: {
                    code.selectAll()
                    code.paste()
                    code.forceActiveFocus()
                }
            }
            Item {
                Layout.fillWidth: true
            }
            UTButton {
                appearance: UTButton.Secondary
                text: qsTr("Cancel")
                onClicked: root.close()
            }
            UTButton {
                text: root.acceptingAnswer ? qsTr("Connect") : qsTr("Accept invitation")
                enabled: code.text.trim().length > 0
                onClicked: root.submit()
            }
        }
    }
}
