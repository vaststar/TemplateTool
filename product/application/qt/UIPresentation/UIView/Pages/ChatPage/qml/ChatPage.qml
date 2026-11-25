import QtQuick
import QtQuick.Layouts
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: root
    property ChatPageController controller: ChatPageController {}

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            ChatConversationList {
                Layout.preferredWidth: Math.max(225, Math.min(300, root.width * 0.32))
                Layout.fillHeight: true
                controller: root.controller
                onInviteRequested: inviteDialog.show(false)
            }

            UTContentDivider {
                Layout.fillHeight: true
                verticalInset: 0
            }

            ChatThread {
                Layout.fillWidth: true
                Layout.fillHeight: true
                controller: root.controller
                onAnswerRequested: inviteDialog.show(true)
            }
        }

        UTText {
            Layout.fillWidth: true
            Layout.margins: 12
            visible: root.controller.error.length > 0 && !inviteDialog.visible
            text: root.controller.error
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
            colorEnum: UIColorToken.Status_Error_Text
        }
    }

    ChatInviteDialog {
        id: inviteDialog
        controller: root.controller
    }
}
