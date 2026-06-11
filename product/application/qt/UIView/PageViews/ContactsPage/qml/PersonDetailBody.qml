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
        text: "基本信息"
        fontEnum: UIFontToken.Body_Text_Medium
        colorEnum: UIColorToken.Content_Text
        Layout.bottomMargin: 4
    }
    DetailFieldRow { label: "ID";   value: root.info ? root.info.id : "" }
    DetailFieldRow { label: "名";   value: root.p.firstName }
    DetailFieldRow { label: "姓";   value: root.p.lastName }
    DetailFieldRow { label: "性别"; value: root.p.gender }
    DetailFieldRow { label: "电话"; value: root.p.phone }
    DetailFieldRow { label: "邮箱"; value: root.p.email }

    Item { Layout.fillHeight: true }
}
