import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import UIResourceLoader 1.0

// Header strip: round avatar (color keyed off kind) + name + kind badge + status text.
RowLayout {
    id: root
    property var info  // expects { name, kind, kindLabel, status }
    signal messageRequested()

    Layout.fillWidth: true
    spacing: 12

    Rectangle {
        width: 44; height: 44; radius: 22
        color: UTComponentUtil.getPlainUIColor(root.info && root.info.kind === "person"
                   ? UIColorToken.Avatar_Person_Background : UIColorToken.Avatar_Group_Background,
                   UIColorState.Normal)

        UTText {
            anchors.centerIn: parent
            text: {
                const n = root.info ? (root.info.name || "") : "";
                return n.length > 0 ? n.substring(0, 1).toUpperCase() : "?";
            }
            fontEnum: UIFontToken.Heading_Text
            colorEnum: UIColorToken.Avatar_Text
        }
    }

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 2

        UTText {
            text: root.info ? (root.info.name || "") : ""
            fontEnum: UIFontToken.Heading_Text
            colorEnum: UIColorToken.Content_Text
            Layout.fillWidth: true
            elide: Text.ElideRight
        }
        RowLayout {
            spacing: 6
            Rectangle {
                Layout.preferredHeight: kindBadge.implicitHeight + 4
                Layout.preferredWidth:  kindBadge.implicitWidth  + 12
                radius: 4
                color: UTComponentUtil.getPlainUIColor(UIColorToken.Listview_Item_Background, UIColorState.Selected)
                UTText {
                    id: kindBadge
                    anchors.centerIn: parent
                    text: root.info ? (root.info.kindLabel || "") : ""
                    fontEnum: UIFontToken.Caption_Text
                    colorEnum: UIColorToken.Link_Text
                }
            }
            UTText {
                text: root.info ? ("· " + (root.info.status || "")) : ""
                fontEnum: UIFontToken.Caption_Text
                colorEnum: UIColorToken.Sidebar_Item_Text
            }
        }
    }

    UTButton {
        visible: root.info && root.info.kind === "person"
        appearance: UTButton.Primary
        text: qsTr("Send message")
        onClicked: root.messageRequested()
    }
}
