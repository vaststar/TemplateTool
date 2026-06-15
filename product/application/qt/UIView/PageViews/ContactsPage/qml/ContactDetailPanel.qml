import QtQuick
import QtQuick.Layouts
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

// Detail panel shell. Owns the empty state, the header strip, the separator, and the
// per-kind body dispatch. All rendering specifics live in the sibling Detail*.qml files
// (DetailHeader / PersonDetailBody / GroupDetailBody / *DetailBody).
Item {
    id: contactDetailPanel

    required property ContactsPageController controller

    readonly property var info: controller ? controller.currentContactInfo : null
    readonly property bool hasInfo: info && info.id !== undefined

    UTText {
        anchors.centerIn: parent
        visible: !contactDetailPanel.hasInfo
        text: "选择联系人查看详情"
        fontEnum: UIFontToken.Body_Text
        colorEnum: UIColorToken.Sidebar_Item_Text
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16
        visible: contactDetailPanel.hasInfo

        DetailHeader {
            Layout.fillWidth: true
            info: contactDetailPanel.info
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: "#1A000000"
        }

        // Body dispatch: Loader keeps the inactive variant unallocated.
        Loader {
            Layout.fillWidth: true
            Layout.fillHeight: true
            sourceComponent: {
                if (!contactDetailPanel.info) return null;
                return contactDetailPanel.info.kind === "person" ? personBody : groupBody;
            }
        }

        Component {
            id: personBody
            PersonDetailBody { info: contactDetailPanel.info }
        }
        Component {
            id: groupBody
            GroupDetailBody { info: contactDetailPanel.info }
        }
    }
}
