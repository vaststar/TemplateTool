import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import UIResourceLoader 1.0

// Group body: always shows the base info section, then data-driven dispatches into
// type-specific sub-bodies based on which sub-map is populated in `info`.
//
// To add a new group type:
//   1) Add a XxxDetailBody.qml that takes a bound sub-map
//   2) Register it in CMakeLists.txt
//   3) Add one line below (it auto-hides via its own `visible:`)
ColumnLayout {
    id: root
    property var info  // full info map

    spacing: 8

    UTText {
        text: qsTr("Basic information")
        fontEnum: UIFontToken.Body_Text_Medium
        colorEnum: UIColorToken.Content_Text
        Layout.bottomMargin: 4
    }
    DetailFieldRow { label: qsTr("ID");   value: root.info ? (root.info.id || "") : "" }
    DetailFieldRow { label: qsTr("Type"); value: root.info ? (root.info.kindLabel || "") : "" }

    // ---- Type-specific sub-bodies. Each is self-hiding via its own `visible:`. ----
    DepartmentDetailBody {
        Layout.fillWidth: true
        Layout.topMargin: 8
        dept: root.info ? (root.info.department || null) : null
    }
    TeamDetailBody {
        Layout.fillWidth: true
        Layout.topMargin: 8
        team: root.info ? (root.info.team || null) : null
    }

    Item { Layout.fillHeight: true }
}
