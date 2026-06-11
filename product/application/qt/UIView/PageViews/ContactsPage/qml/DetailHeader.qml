import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import UIResourceLoader 1.0

// Header strip: round avatar (color keyed off kind) + name + kind badge + status text.
RowLayout {
    id: root
    property var info  // expects { name, kind, kindLabel, status }

    Layout.fillWidth: true
    spacing: 12

    Rectangle {
        width: 44; height: 44; radius: 22
        // Person uses blue, all groups use purple. Add more kind->color cases here when
        // you want per-group-type accents.
        color: root.info && root.info.kind === "person" ? "#3F8AE0" : "#7E5BEF"

        UTText {
            anchors.centerIn: parent
            text: {
                const n = root.info ? (root.info.name || "") : "";
                return n.length > 0 ? n.substring(0, 1).toUpperCase() : "?";
            }
            fontEnum: UIFontToken.Heading_Text
            color: "white"
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
                color: "#E8EEF7"
                UTText {
                    id: kindBadge
                    anchors.centerIn: parent
                    text: root.info ? (root.info.kindLabel || "") : ""
                    fontEnum: UIFontToken.Caption_Text
                    color: "#3F5772"
                }
            }
            UTText {
                text: root.info ? ("· " + (root.info.status || "")) : ""
                fontEnum: UIFontToken.Caption_Text
                colorEnum: UIColorToken.Sidebar_Item_Text
            }
        }
    }
}
