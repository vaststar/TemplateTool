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
        text: "部门信息"
        fontEnum: UIFontToken.Body_Text_Medium
        colorEnum: UIColorToken.Content_Text
        Layout.bottomMargin: 4
    }
    DetailFieldRow {
        label: "负责人"
        value: root.dept
            ? (root.dept.managerName && root.dept.managerName.length > 0
               ? root.dept.managerName + "（" + root.dept.managerId + "）"
               : root.dept.managerId)
            : ""
    }
    DetailFieldRow {
        label: "人数"
        value: root.dept ? String(root.dept.headcount) : ""
    }
}
