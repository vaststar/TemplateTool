import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import UIResourceLoader 1.0

// Person body: base info + profile fields. Empty fields auto-hide via DetailFieldRow.
ColumnLayout {
    id: root
    property var info  // full info map (contains .person sub-map)

    readonly property var p: root.info && root.info.person ? root.info.person : ({})

    spacing: 8

    UTText {
        text: qsTr("Basic information")
        fontEnum: UIFontToken.Body_Text_Medium
        colorEnum: UIColorToken.Content_Text
        Layout.bottomMargin: 4
    }
    DetailFieldRow { label: qsTr("ID");   value: root.info ? root.info.id : "" }
    DetailFieldRow { label: qsTr("First name");   value: root.p.firstName }
    DetailFieldRow { label: qsTr("Last name");   value: root.p.lastName }
    DetailFieldRow { label: qsTr("Gender"); value: root.p.gender }
    DetailFieldRow { label: qsTr("Phone"); value: root.p.phone }
    DetailFieldRow { label: qsTr("Email"); value: root.p.email }

    Item { Layout.fillHeight: true }
}
