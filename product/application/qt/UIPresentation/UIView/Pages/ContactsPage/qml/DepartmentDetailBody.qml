import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import UIResourceLoader 1.0

// Department-specific section. Bound to info.department (already a QVariantMap with
// managerId / managerName / headcount).
ColumnLayout {
    id: root
    property var dept

    spacing: 8
    visible: !!root.dept

    UTText {
        text: qsTr("Department information")
        fontEnum: UIFontToken.Body_Text_Medium
        colorEnum: UIColorToken.Content_Text
        Layout.bottomMargin: 4
    }
    DetailFieldRow {
        label: qsTr("Manager")
        value: root.dept
            ? (root.dept.managerName && root.dept.managerName.length > 0
               ? qsTr("%1 (%2)").arg(root.dept.managerName).arg(root.dept.managerId)
               : root.dept.managerId)
            : ""
    }
    DetailFieldRow {
        label: qsTr("Headcount")
        value: root.dept ? String(root.dept.headcount) : ""
    }
}
