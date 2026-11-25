import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: root
    required property ChatPageController controller
    signal answerRequested()

    readonly property string activeId: controller.conversation.id || ""
    readonly property string activeName: controller.conversation.name || ""
    readonly property int connectionState: controller.connectionState
    readonly property bool connected: connectionState === ChatPageController.Connected
    readonly property bool canSend: controller.ready && controller.conversation.isVerified === true
    property var drafts: ({})
    property var replyDrafts: ({})
    property string replyToMessageId: ""
    property string highlightedMessageId: ""
    property bool followMessages: true

    readonly property var draftReply: {
        const messages = controller.messages
        const index = controller.messageIndex(replyToMessageId)
        return index >= 0 ? messages[index] || null : null
    }

    component QuoteButton: UTButton {
        id: quote
        required property bool available
        required property string label
        required property string excerpt

        appearance: UTButton.Ghost
        radius: 5
        leftPadding: 4
        rightPadding: 4
        topPadding: 4
        bottomPadding: 4
        enabled: available
        Accessible.name: available ? qsTr("Jump to original message") : quoteExcerpt.text
        Accessible.description: available ? label + ", " + quoteExcerpt.text : ""

        contentItem: RowLayout {
            spacing: 6
            Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: 2
                color: UTComponentUtil.getPlainUIColor(
                           quote.available ? UIColorToken.Link_Text : UIColorToken.Window_Border,
                           UIColorState.Normal)
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2
                UTText {
                    Layout.fillWidth: true
                    visible: quote.label.length > 0
                    text: quote.label
                    textFormat: Text.PlainText
                    fontEnum: UIFontToken.Caption_Text
                    elide: Text.ElideRight
                }
                UTText {
                    id: quoteExcerpt
                    Layout.fillWidth: true
                    text: quote.available ? quote.excerpt.replace(/\s+/g, " ")
                                          : qsTr("Original message unavailable")
                    textFormat: Text.PlainText
                    fontEnum: UIFontToken.Caption_Text
                    colorEnum: UIColorToken.Content_Secondary_Text
                    elide: Text.ElideRight
                }
            }
        }
    }

    function statusText() {
        switch (connectionState) {
        case ChatPageController.PreparingInvite: return qsTr("Preparing invitation…")
        case ChatPageController.AwaitingAnswer: return qsTr("Waiting for reply")
        case ChatPageController.Connecting: return qsTr("Connecting…")
        case ChatPageController.VerifyPeer: return qsTr("Verify device")
        case ChatPageController.Connected: return qsTr("Connected")
        case ChatPageController.Failed: return qsTr("Connection failed")
        default: return qsTr("Disconnected")
        }
    }

    function sendMessage() {
        if (canSend && composer.text.trim().length && controller.sendText(composer.text, replyToMessageId)) {
            delete drafts[activeId]
            composer.clear()
            clearReplyDraft()
            followMessages = true
            Qt.callLater(scrollToLatest)
        }
    }

    function startReply(messageId) {
        if (canSend && controller.messageIndex(messageId) >= 0) {
            replyDrafts[activeId] = messageId
            replyToMessageId = messageId
            composer.forceActiveFocus()
        }
    }

    function clearReplyDraft() {
        delete replyDrafts[activeId]
        replyToMessageId = ""
    }

    function jumpToMessage(messageId) {
        const index = controller.messageIndex(messageId)
        if (index >= 0) {
            followMessages = false
            messageList.positionViewAtIndex(index, ListView.Center)
            highlightedMessageId = messageId
            highlightTimer.restart()
        }
    }

    function scrollToLatest() {
        if (followMessages)
            messageList.positionViewAtEnd()
    }

    onActiveIdChanged: {
        composer.text = drafts[activeId] || ""
        replyToMessageId = replyDrafts[activeId] || ""
        copyTimer.stop()
        highlightTimer.stop()
        highlightedMessageId = ""
        followMessages = true
        Qt.callLater(scrollToLatest)
    }

    UTText {
        anchors.centerIn: parent
        width: Math.min(parent.width - 32, implicitWidth)
        visible: !root.activeId.length
        text: !root.controller.ready ? qsTr("Loading chat…")
             : root.controller.rows.length ? qsTr("Select a conversation.")
             : qsTr("Open a person in Contacts to start a chat, or accept an invitation.")
        colorEnum: UIColorToken.Content_Secondary_Text
        wrapMode: Text.Wrap
        horizontalAlignment: Text.AlignHCenter
    }

    ColumnLayout {
        anchors.fill: parent
        visible: root.activeId.length > 0
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.margins: 14
            spacing: 10

            Rectangle {
                Layout.preferredWidth: 40
                Layout.preferredHeight: 40
                radius: 20
                color: UTComponentUtil.getPlainUIColor(UIColorToken.Avatar_Person_Background, UIColorState.Normal)
                UTText {
                    anchors.centerIn: parent
                    text: root.activeName.slice(0, 1).toUpperCase()
                    textFormat: Text.PlainText
                    colorEnum: UIColorToken.Avatar_Text
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 3
                UTText {
                    Layout.fillWidth: true
                    text: root.activeName
                    textFormat: Text.PlainText
                    fontEnum: UIFontToken.Section_Title
                    colorEnum: UIColorToken.Content_Heading
                    elide: Text.ElideRight
                }
                UTText {
                    Layout.fillWidth: true
                    text: root.statusText()
                    fontEnum: UIFontToken.Caption_Text
                    colorEnum: root.connected ? UIColorToken.Status_Success_Text
                                              : UIColorToken.Content_Secondary_Text
                    elide: Text.ElideRight
                }
            }
            UTButton {
                id: groupButton
                appearance: UTButton.Secondary
                radius: 6
                text: qsTr("Move to group")
                enabled: root.controller.ready
                onClicked: groupMenu.popup(groupButton, 0, height)
            }
            UTButton {
                appearance: UTButton.Ghost
                radius: 6
                visible: root.connected
                text: qsTr("Disconnect")
                onClicked: root.controller.disconnectPeer()
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: UTComponentUtil.getPlainUIColor(UIColorToken.Window_Border, UIColorState.Normal)
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.margins: 14
            implicitHeight: connectionContent.implicitHeight + 24
            visible: !root.connected || root.controller.connectionError.length > 0
            radius: 8
            color: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Background, UIColorState.Normal)
            border.color: UTComponentUtil.getPlainUIColor(UIColorToken.Window_Border, UIColorState.Normal)

            ColumnLayout {
                id: connectionContent
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                UTText {
                    Layout.fillWidth: true
                    visible: text.length > 0
                    text: {
                        switch (root.connectionState) {
                        case ChatPageController.PreparingInvite:
                            return qsTr("Preparing the information your contact needs to connect.")
                        case ChatPageController.AwaitingAnswer:
                            return qsTr("Send the invitation through a channel you already use. When your contact sends back a reply, paste it here.")
                        case ChatPageController.Connecting:
                            return root.controller.answerAvailable
                                ? qsTr("Send this reply to the person who sent the invitation.")
                                : qsTr("Waiting for the other device to connect.")
                        case ChatPageController.VerifyPeer:
                            return qsTr("Compare these fingerprints with your contact through a trusted channel. Your fingerprint should match the one they see for you, and theirs should match the one below.")
                        case ChatPageController.Disconnected:
                        case ChatPageController.Failed:
                            return qsTr("Create an invitation to connect when both devices are online.")
                        default:
                            return ""
                        }
                    }
                    wrapMode: Text.WordWrap
                }
                UTText {
                    Layout.fillWidth: true
                    visible: root.controller.connectionError.length > 0
                    text: root.controller.connectionError
                    textFormat: Text.PlainText
                    wrapMode: Text.Wrap
                    colorEnum: UIColorToken.Status_Error_Text
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    visible: root.connectionState === ChatPageController.VerifyPeer
                    spacing: 4

                    UTText {
                        text: qsTr("Your fingerprint")
                        fontEnum: UIFontToken.Body_Text_Medium
                    }
                    UTText {
                        Layout.fillWidth: true
                        text: root.controller.localFingerprint
                        textFormat: Text.PlainText
                        fontEnum: UIFontToken.Monospace_Text
                        wrapMode: Text.WrapAnywhere
                    }
                    UTText {
                        text: qsTr("Their fingerprint")
                        fontEnum: UIFontToken.Body_Text_Medium
                    }
                    UTText {
                        Layout.fillWidth: true
                        text: root.controller.remoteFingerprint
                        textFormat: Text.PlainText
                        fontEnum: UIFontToken.Monospace_Text
                        wrapMode: Text.WrapAnywhere
                    }
                }

                Flow {
                    Layout.fillWidth: true
                    spacing: 6

                    UTButton {
                        visible: root.connectionState === ChatPageController.Disconnected
                                 || root.connectionState === ChatPageController.Failed
                        text: qsTr("Create invitation")
                        onClicked: root.controller.createInvite()
                    }
                    UTButton {
                        visible: root.connectionState === ChatPageController.AwaitingAnswer
                        enabled: root.controller.inviteAvailable
                        text: qsTr("Copy invitation")
                        onClicked: {
                            if (root.controller.copyInvite()) {
                                copyFeedback.text = qsTr("Invitation copied")
                                copyTimer.restart()
                            }
                        }
                    }
                    UTButton {
                        visible: root.connectionState === ChatPageController.AwaitingAnswer
                        appearance: UTButton.Secondary
                        text: qsTr("Paste reply")
                        onClicked: root.answerRequested()
                    }
                    UTButton {
                        visible: root.connectionState === ChatPageController.Connecting && root.controller.answerAvailable
                        text: qsTr("Copy reply")
                        onClicked: {
                            if (root.controller.copyAnswer()) {
                                copyFeedback.text = qsTr("Reply copied")
                                copyTimer.restart()
                            }
                        }
                    }
                    UTButton {
                        visible: root.connectionState === ChatPageController.VerifyPeer
                        enabled: root.controller.localFingerprint.length > 0 && root.controller.remoteFingerprint.length > 0
                        text: qsTr("Fingerprints match")
                        onClicked: root.controller.verifyPeer()
                    }
                    UTButton {
                        visible: root.connectionState === ChatPageController.PreparingInvite
                                 || root.connectionState === ChatPageController.AwaitingAnswer
                                 || root.connectionState === ChatPageController.Connecting
                                 || root.connectionState === ChatPageController.VerifyPeer
                        appearance: UTButton.Secondary
                        text: qsTr("Cancel")
                        onClicked: root.controller.disconnectPeer()
                    }
                }

                UTText {
                    id: copyFeedback
                    visible: copyTimer.running
                    fontEnum: UIFontToken.Caption_Text
                    colorEnum: UIColorToken.Content_Secondary_Text
                }
                Timer {
                    id: copyTimer
                    interval: 2300
                }
            }
        }

        ListView {
            id: messageList
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: 16
            Layout.rightMargin: 16
            Layout.minimumHeight: 0
            clip: true
            spacing: 10
            boundsBehavior: Flickable.StopAtBounds
            model: root.controller.messages
            ScrollBar.vertical: UTScrollBar {}
            onMovementEnded: root.followMessages = atYEnd
            onCountChanged: {
                if (root.followMessages)
                    Qt.callLater(root.scrollToLatest)
            }

            delegate: ColumnLayout {
                id: messageRow
                required property var modelData
                width: ListView.view.width
                spacing: 4

                UTText {
                    Layout.alignment: Qt.AlignHCenter
                    visible: messageRow.modelData.day.length > 0
                    text: messageRow.modelData.day
                    fontEnum: UIFontToken.Caption_Text
                    colorEnum: UIColorToken.Content_Secondary_Text
                }
                Rectangle {
                    id: bubble
                    Layout.alignment: messageRow.modelData.outgoing ? Qt.AlignRight : Qt.AlignLeft
                    implicitWidth: Math.min(messageRow.width * 0.78,
                                            Math.max(messageText.implicitWidth + 20, messageQuote.visible ? 220 : 0))
                    implicitHeight: messageText.implicitHeight + 16
                                    + (messageQuote.visible ? messageQuote.implicitHeight + 6 : 0)
                    radius: 10
                    border.width: root.highlightedMessageId === messageRow.modelData.id ? 2 : 0
                    border.color: UTComponentUtil.getPlainUIColor(UIColorToken.Link_Text, UIColorState.Normal)
                    color: UTComponentUtil.getPlainUIColor(
                               messageRow.modelData.outgoing ? UIColorToken.Content_Nav_Item_Background
                                                             : UIColorToken.Content_Section_Background,
                               messageRow.modelData.outgoing ? UIColorState.Selected : UIColorState.Normal)
                    QuoteButton {
                        id: messageQuote
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: 8
                        visible: messageRow.modelData.replyToMessageId.length > 0
                        available: messageRow.modelData.replyAvailable
                        label: !available ? "" : messageRow.modelData.replyOutgoing ? qsTr("You") : root.activeName
                        excerpt: messageRow.modelData.replyText
                        onClicked: root.jumpToMessage(messageRow.modelData.replyToMessageId)
                    }
                    UTText {
                        id: messageText
                        anchors.top: messageQuote.visible ? messageQuote.bottom : parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        anchors.topMargin: messageQuote.visible ? 6 : 8
                        text: messageRow.modelData.text
                        textFormat: Text.PlainText
                        wrapMode: Text.Wrap
                    }
                    TapHandler {
                        enabled: root.canSend
                        acceptedButtons: Qt.RightButton
                        onTapped: point => {
                            replyMenu.messageId = messageRow.modelData.id
                            replyMenu.popup(bubble, point.position.x, point.position.y)
                        }
                    }
                }
                RowLayout {
                    Layout.alignment: messageRow.modelData.outgoing ? Qt.AlignRight : Qt.AlignLeft
                    spacing: 5
                    UTText {
                        text: messageRow.modelData.outgoing
                            ? qsTr("%1 · %2").arg(messageRow.modelData.time)
                                 .arg(messageRow.modelData.delivered ? qsTr("Delivered") : qsTr("Pending"))
                            : messageRow.modelData.time
                        fontEnum: UIFontToken.Caption_Text
                        colorEnum: UIColorToken.Content_Secondary_Text
                    }
                    UTButton {
                        visible: root.canSend
                        appearance: UTButton.Ghost
                        fontEnum: UIFontToken.Caption_Text
                        fontColorEnum: UIColorToken.Link_Text
                        topPadding: 1
                        bottomPadding: 1
                        leftPadding: 3
                        rightPadding: 3
                        text: qsTr("Reply")
                        onClicked: root.startReply(messageRow.modelData.id)
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 14
            Layout.rightMargin: 14
            Layout.topMargin: 8
            spacing: 8
            visible: root.replyToMessageId.length > 0

            QuoteButton {
                Layout.fillWidth: true
                available: root.draftReply !== null
                label: available ? qsTr("Replying to %1").arg(root.draftReply.outgoing ? qsTr("yourself") : root.activeName)
                                 : qsTr("Replying to a message")
                excerpt: available ? root.draftReply.text : ""
                onClicked: root.jumpToMessage(root.replyToMessageId)
            }
            UTButton {
                appearance: UTButton.Ghost
                text: "×"
                Accessible.name: qsTr("Cancel reply")
                onClicked: {
                    root.clearReplyDraft()
                    composer.forceActiveFocus()
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.margins: 14
            spacing: 8
            UTTextField {
                id: composer
                Layout.fillWidth: true
                enabled: root.canSend
                Accessible.name: qsTr("Message")
                placeholderText: root.connected ? qsTr("Write a message…")
                                 : root.canSend ? qsTr("Write a message to send when connected…")
                                 : qsTr("Verify the device to send a message")
                onTextEdited: root.drafts[root.activeId] = text
                onAccepted: root.sendMessage()
            }
            UTButton {
                text: qsTr("Send")
                radius: 6
                enabled: root.canSend && composer.text.trim().length > 0
                onClicked: root.sendMessage()
            }
        }
    }

    UTDynamicMenu {
        id: groupMenu
        model: root.controller.groupOptions.map(group => ({text: group.name, action: "group:" + group.id}))
        onItemTriggered: action => root.controller.moveConversation(action.slice(6))
    }

    UTDynamicMenu {
        id: replyMenu
        property string messageId
        model: [{text: qsTr("Reply"), action: "reply"}]
        onItemTriggered: root.startReply(replyMenu.messageId)
    }

    Timer {
        id: highlightTimer
        interval: 1800
        onTriggered: root.highlightedMessageId = ""
    }
}
